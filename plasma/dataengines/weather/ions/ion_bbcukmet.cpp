/***************************************************************************
 *   Copyright (C) 2007 by Shawn Starr <shawn.starr@rogers.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

/* Ion for BBC's Weather from the UK Met Office */

#include "ion_bbcukmet.h"

class UKMETIon::Private : public QObject
{
public:
    Private() {}
    ~Private() {}

private:
    struct XMLMapInfo {
        QString place;
        QString XMLurl;
        bool ukPlace;
    };

public:
    // Key dicts
    QHash<QString, UKMETIon::Private::XMLMapInfo> m_place;
    QVector<QString> m_locations;
    QStringList m_matchLocations;
    bool isValid;
public:
    // Weather information
    QHash<QString, WeatherData> m_weatherData;

    // Store KIO jobs - Search list
    QMap<KJob *, QXmlStreamReader*> m_jobXml;
    QMap<KJob *, QString> m_jobList;

    QMap<KJob *, QXmlStreamReader*> m_forecastJobXml;
    QMap<KJob *, QString> m_forecastJobList;

    
    KUrl *m_url;
    KIO::TransferJob *m_job;

    bool m_useUTC;  // Ion option: Timezone may be local time or UTC time
    bool m_useMetric; // Ion option: Units may be Metric or Imperial
    bool m_windInMeters; // Ion option: Display wind format in meters per second only

    WeatherFormula m_formula;
};


// ctor, dtor
UKMETIon::UKMETIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent), d(new Private())

{
    Q_UNUSED(args)
}

UKMETIon::~UKMETIon()
{
    // Destroy dptr
    delete d;
}

// Get the master list of locations to be parsed
void UKMETIon::init()
{
    this->setInitialized(true);
}

// Get a specific Ion's data
bool UKMETIon::updateIonSource(const QString& source)
{
    // We expect the applet to send the source in the following tokenization:
    // ionname|validate|place_name - Triggers validation of place
    // ionname|weather|place_name - Triggers receiving weather of place

    kDebug() << "updateIonSource() SOURCE: " << source;
    QStringList sourceAction = source.split('|');

    if (sourceAction[1] == QString("validate")) {
        kDebug() << "Initiate Find Matching places: " << sourceAction[2];
        // Look for places to match
        this->findPlace(sourceAction[2], source);
        return true;

    } else if (sourceAction[1] == QString("weather")) {
       d->m_place[QString("bbcukmet|%1").arg(sourceAction[2])].XMLurl = sourceAction[3]; 
       getXMLData(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));
       return true;
    }
    return false;
}

// Gets specific city XML data
void UKMETIon::getXMLData(const QString& source)
{
    KUrl url;
   
    url = d->m_place[source].XMLurl;

    kDebug() << "URL Location: " << url.url();

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_job->addMetaData("cookies", "none"); // Disable displaying cookies
    d->m_forecastJobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_forecastJobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(forecast_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(forecast_slotJobFinished(KJob *)));
    }
}

// Parses city list and gets the correct city based on ID number
void UKMETIon::findPlace(const QString& place, const QString& source)
{
    d->m_jobList.clear();
    d->m_jobXml.clear();

    KUrl url;
    url = "http://www.bbc.co.uk/cgi-perl/weather/search/new_search.pl?x=0&y=0&=Submit&search_query=" + place + "&tmpl=wap";
    kDebug() << "URL: " << url;

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_job->addMetaData("cookies", "none"); // Disable displaying cookies

    d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_jobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(setup_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(setup_slotJobFinished(KJob *)));
    }
}

bool UKMETIon::readSearchXMLData(const QString& key, QXmlStreamReader& xml)
{
    kDebug() << "readSearchXMLData()";

    while (!xml.atEnd()) {
        xml.readNext();
 
        if (xml.isEndElement()) {
            break;
        }
 
        if (xml.isStartElement()) {
            kDebug() << "XML TAG: " << xml.name().toString();
            if (xml.name() == "wml") {
                parseSearchLocations(key, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

return !xml.error();
}

void UKMETIon::parseSearchLocations(const QString& source, QXmlStreamReader& xml)
{ 
    int flag = 0;
    QString url;
    QString place;
    QStringList tokens;
    QString tmp;
    int counter = 2;
    Q_ASSERT(xml.isStartElement() && xml.name() == "wml");
    d->m_locations.clear();
    while (!xml.atEnd()) {
        xml.readNext();
  
        if (xml.isEndElement() && xml.name() == "wml") {
           break;
        }
     
        if (xml.isStartElement()) {
            if (xml.name() == "a" && !xml.attributes().value("href").isEmpty()) {
                if (xml.attributes().value("href").toString().contains("5day.wml")) {

                    // Split URL to determine station ID number
                    tokens = xml.attributes().value("href").toString().split("=");
                    if (xml.attributes().value("href").toString().contains("world")) {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/world/" + tokens[1] + ".xml";
                        flag = 0;
                    } else {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/id/" + tokens[1] + ".xml";
                        flag = 1;
                    }
                    place = xml.readElementText();
                    tmp = QString("bbcukmet|%1").arg(place);
 
                    kDebug() << "PLACES FOUND: " << place; 
                    kDebug() << "URL FOR PLACE: " << url;

                    // Duplicate places can exist
                    if (d->m_locations.contains(tmp)) {
                         
                        QString dupePlace = place;
                        tmp = QString("bbcukmet|%1").arg(QString("%1 (#%2)").arg(dupePlace).arg(counter));
                        place = QString("%1 (#%2)").arg(dupePlace).arg(counter);
                        counter++;
                    }
                    
                        if (flag) {  // This is a UK specific location
                            d->m_place[tmp].XMLurl = url;
                            d->m_place[tmp].place = place;
                            d->m_place[tmp].ukPlace = true;
                        } else {
                            d->m_place[tmp].XMLurl = url;
                            d->m_place[tmp].place = place;
                            d->m_place[tmp].ukPlace = false;
                        }
                        d->m_locations.append(tmp);
                }
            }
        } 
    }
    validate(source);
}

// handle when no XML tag is found
void UKMETIon::parseUnknownElement(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            parseUnknownElement(xml);
        }
    }
}
    
void UKMETIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    kDebug() << "JOB ERROR(): " << job->errorString();

    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data.data());
}

void UKMETIon::setup_slotJobFinished(KJob *job)
{
    if (job->error() == 149) {
        kDebug() << "JOB ERROR: " << job->errorString(); 
        setData(d->m_jobList[job], "validate", QString("bbcukmet|timeout"));
        disconnectSource(d->m_jobList[job], this);
        d->m_jobList.remove(job);
        delete d->m_jobXml[job];
        d->m_jobXml.remove(job);
        return;
    }   
    readSearchXMLData(d->m_jobList[job], *d->m_jobXml[job]);
    d->m_jobList.remove(job);
    delete d->m_jobXml[job];
    d->m_jobXml.remove(job);
}

void UKMETIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    kDebug() << "UKMET: RECEIVING FORECAST INFORMATION\n";
    if (data.isEmpty() || !d->m_forecastJobXml.contains(job)) {
        return;
    }
  
    // Send to xml.
    d->m_forecastJobXml[job]->addData(data.data());
}

void UKMETIon::forecast_slotJobFinished(KJob *job)
{
    kDebug() << "UKMET: FORECAST INFO FOR " << d->m_forecastJobList[job] << " FINISHED\n";
    readObservationXMLData(d->m_forecastJobList[job], *d->m_forecastJobXml[job]);
    d->m_forecastJobList.remove(job);
    delete d->m_forecastJobXml[job];
    d->m_forecastJobXml.remove(job);
}

void UKMETIon::parsePlaceObservation(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "rss");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "rss") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "channel") {
                parseWeatherChannel(data, xml);
            }
        }
    }
}

void UKMETIon::parseWeatherChannel(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "channel");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "channel") {
            break;
        }
  
        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                data.stationName = xml.readElementText().split("Observations for")[1].trimmed();
            } else if (xml.name() == "item") {
                parseWeatherObservation(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void UKMETIon::parseWeatherObservation(WeatherData& data, QXmlStreamReader& xml)
{   
    Q_UNUSED(data)
    Q_ASSERT(xml.isStartElement() && xml.name() == "item");
 
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "item") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                QString conditionString = xml.readElementText();

                // Get the observation time.
                QStringList conditionData = conditionString.split(":");

                data.obsTime = conditionData[0];
                data.condition = conditionData[1].split(".")[0].trimmed();
                kDebug() << "TIME: " << data.obsTime;
	        kDebug() << "CONDITIONS: " << data.condition;

            } else if (xml.name() == "description") {
                QString observeString = xml.readElementText();
                QStringList observeData = observeString.split(":");

                data.temperature_C = observeData[1].split(QChar(176))[0].trimmed();
                data.temperature_F = observeData[1].split("(")[1].split(QChar(176))[0];

                data.windDirection = observeData[2].split(",")[0].trimmed();
                data.windSpeed_miles = observeData[3].split(",")[0].split(" ")[1];
        
                data.humidity = observeData[4].split(",")[0].split(" ")[1];

                data.pressure = observeData[5].split(",")[0].split(" ")[1].split("mB")[0];

                data.pressureTendency = observeData[5].split(",")[1].trimmed();

                data.visibilityStr = observeData[6].trimmed();
                kDebug() << "TEMPERATURE IN C: " << data.temperature_C;
                kDebug() << "TEMPERATURE IN F: " << data.temperature_F;

                kDebug() << "WIND DIRECTION:" << data.windDirection;
                kDebug() << "WIND SPEED: " << data.windSpeed_miles;
                kDebug() << "HUMIDITY: " << data.humidity;
 
                kDebug() << "PRESSURE: " << data.pressure;
                kDebug() << "PRESSURE TENDENCY: " << data.pressureTendency;
                kDebug() << "VISIBILITY: " << data.visibilityStr;

            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

bool UKMETIon::readObservationXMLData(QString& source, QXmlStreamReader& xml)
{
    WeatherData data;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            kDebug() << "XML TAG: " << xml.name().toString();
            if (xml.name() == "rss") {
                parsePlaceObservation(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    kDebug() << "SOURCE ======>" << source;
    d->m_weatherData[source] = data;
    updateWeather(source);
    return !xml.error();
}

// User toggleable values set from the dataengine <-> Plasma Applet
void UKMETIon::option(int option, const QVariant& value)
{
    switch (option) {
    case IonInterface::UNITS:
        // Set the Units used (Depends on Ion)
        if (value.toInt() == KLocale::Metric) {
            d->m_useMetric = true;
        }
        if (value.toInt() == KLocale::Imperial) {
            d->m_useMetric = false;
        }
        break;
    case IonInterface::TIMEFORMAT:
        if (value.toBool()) {
            d->m_useUTC = true;
        }
        break;
    case IonInterface::WINDFORMAT:
        if (value.toBool()) {
            d->m_windInMeters = true;
        } else {
            d->m_windInMeters = false;
        }
        break;
    }
}

void UKMETIon::validate(const QString& source)
{
    bool beginflag = true;

    if (!d->m_locations.count()) {
        QStringList invalidPlace = source.split('|');
        setData(source, "validate", QString("bbcukmet|invalid|multiple|%1").arg(invalidPlace[2]));
        return;
    } else {
        QString placeList;
        foreach (QString place, d->m_locations) {
                 if (beginflag) {
                     placeList.append(QString("%1|extra|%2").arg(place.split("|")[1]).arg(d->m_place[place].XMLurl));
                     beginflag = false;
                 } else {
                     placeList.append(QString("|place|%1|extra|%2").arg(place.split("|")[1]).arg(d->m_place[place].XMLurl));
                 }
        }
        setData(source, "validate", QString("bbcukmet|valid|multiple|place|%1").arg(placeList));
    }
}

void UKMETIon::updateWeather(const QString& source)
{
    QString weatherSource = source;
    weatherSource.replace("bbcukmet|", "bbcukmet|weather|");
    weatherSource.append(QString("|%1").arg(d->m_place[source].XMLurl));

    QMap<QString, QString> dataFields;
    QStringList fieldList;

    setData(weatherSource, "Place", this->place(source));
    setData(weatherSource, "Station", this->station(source));
    setData(weatherSource, "Observation Period", this->observationTime(source));
    setData(weatherSource, "Current Conditions", this->condition(source));
 
    setData(weatherSource, "Humidity", this->humidity(source));
    setData(weatherSource, "Visibility", this->visibility(source));
   
    dataFields = this->temperature(source);
    setData(weatherSource, "Temperature", dataFields["temperature"]);
    setData(weatherSource, "Temperature Unit", dataFields["temperatureUnit"]);
   
    dataFields = this->pressure(source);
    setData(weatherSource, "Pressure", dataFields["pressure"]);
    setData(weatherSource, "Pressure Unit", dataFields["pressureUnit"]);
    setData(weatherSource, "Pressure Tendency", dataFields["pressureTendency"]);
  
    dataFields = this->wind(source);
    setData(weatherSource, "Wind Speed", dataFields["windSpeed"]);
    setData(weatherSource, "Wind Speed Unit", dataFields["windUnit"]);
    setData(weatherSource, "Wind Direction", dataFields["windDirection"]);

    setData(weatherSource, "Credit", "BBC / UK MET Office");
}

QString UKMETIon::place(const QString& source)
{
    return source.split("|")[1];
}

QString UKMETIon::station(const QString& source)
{
    return d->m_weatherData[source].stationName;
}

QString UKMETIon::observationTime(const QString& source)
{
    return d->m_weatherData[source].obsTime;
}

QString UKMETIon::condition(const QString& source)
{
    return d->m_weatherData[source].condition;
}

QMap<QString, QString> UKMETIon::temperature(const QString& source)
{
    QMap<QString, QString> temperatureInfo;

    if (d->m_useMetric) {
        temperatureInfo.insert("temperature", QString("%1").arg(d->m_weatherData[source].temperature_C));
        temperatureInfo.insert("temperatureUnit", QString("%1C").arg(QChar(176)));
    } else {
        temperatureInfo.insert("temperature", QString("%1").arg(d->m_weatherData[source].temperature_F));
        temperatureInfo.insert("temperatureUnit", QString("%1F").arg(QChar(176)));
    }
    return temperatureInfo;
}

QMap<QString, QString> UKMETIon::wind(const QString& source)
{
    QMap<QString, QString> windInfo;
    if (d->m_weatherData[source].windSpeed_miles == "N/A") {
        windInfo.insert("windSpeed", "N/A");
        windInfo.insert("windUnit", "N/A");
    } else {
        if (d->m_useMetric) {
            if (d->m_windInMeters) {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.milesToMS(d->m_weatherData[source].windSpeed_miles.toFloat()), 'f', 2)));
                windInfo.insert("windUnit","m/s");
            } else {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.milesToKM(d->m_weatherData[source].windSpeed_miles.toFloat()), 'f', 1)));
                windInfo.insert("windUnit", "km/h");
            } 
        } else {
            windInfo.insert("windSpeed", QString(d->m_weatherData[source].windSpeed_miles));
            windInfo.insert("windUnit", "mph");
        }
    }
    windInfo.insert("windDirection", d->m_weatherData[source].windDirection);
    return windInfo;
}

QString UKMETIon::humidity(const QString& source)
{
    if (d->m_weatherData[source].humidity == "N/A%") {
        return "N/A";
    } 
    return d->m_weatherData[source].humidity;
}

QString UKMETIon::visibility(const QString& source)
{
    return d->m_weatherData[source].visibilityStr;
}

QMap<QString, QString> UKMETIon::pressure(const QString& source)
{
    QMap<QString, QString> pressureInfo;
    if (d->m_weatherData[source].pressure == "N/A") {
        pressureInfo.insert("pressure", "N/A");
        return pressureInfo;
    }
   
    if (d->m_useMetric) {
        pressureInfo.insert("pressure", QString("%1").arg(QString::number(d->m_formula.millibarsToKilopascals(d->m_weatherData[source].pressure.toFloat()), 'f', 1)));
        pressureInfo.insert("pressureUnit", "kPa");
    } else {
        pressureInfo.insert("pressure", QString("%1").arg(QString::number(d->m_formula.millibarsToInches(d->m_weatherData[source].pressure.toFloat()), 'f', 2)));
        pressureInfo.insert("pressureUnit", "in");
    }

    pressureInfo.insert("pressureTendency", d->m_weatherData[source].pressureTendency);
    return pressureInfo;
}

#include "ion_bbcukmet.moc"

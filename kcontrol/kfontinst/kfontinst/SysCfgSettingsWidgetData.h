/****************************************************************************
** Form interface generated from reading ui file 'SysCfgSettingsWidget.ui'
**
** Created: Wed Nov 28 22:24:44 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CSYSCFGSETTINGSWIDGETDATA_H
#define CSYSCFGSETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;

class CSysCfgSettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CSysCfgSettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CSysCfgSettingsWidgetData();

    QButtonGroup* ButtonGroup5_2_2;
    QRadioButton* itsXsetRadio;
    QRadioButton* itsXfsRadio;
    QRadioButton* itsCustomRadio;
    QLineEdit* itsRestartXfsCommand;
    QGroupBox* GroupBox4_2;
    QCheckBox* itsX11EncodingCheck;
    QComboBox* itsX11EncodingCombo;
    QLabel* TextLabel1;
    QComboBox* itsAfmEncodingCombo;
    QCheckBox* itsT1AfmCheck;
    QCheckBox* itsGenAfmsCheck;
    QLabel* TextLabel1_2;
    QCheckBox* itsTtAfmCheck;
    QCheckBox* itsOverwriteAfmsCheck;


public slots:
    virtual void afmEncodingSelected(const QString &);
    virtual void customXRefreshSelected(bool);
    virtual void customXStrChanged(const QString &);
    virtual void encodingSelected(bool);
    virtual void encodingSelected(const QString &);
    virtual void generateAfmsSelected(bool);
    virtual void overwriteAfmsSelected(bool);
    virtual void t1AfmSelected(bool);
    virtual void ttAfmSelected(bool);
    virtual void xfsRestartSelected(bool);
    virtual void xsetFpRehashSelected(bool);

protected:
    QGridLayout* CSysCfgSettingsWidgetDataLayout;
    QGridLayout* ButtonGroup5_2_2Layout;
    QGridLayout* GroupBox4_2Layout;
};

#endif // CSYSCFGSETTINGSWIDGETDATA_H


########### next target ###############
set( oxygen_demo_SOURCES
    oxygenbenchmarkwidget.cpp
    oxygenbuttondemowidget.cpp
    oxygendemodialog.cpp
    oxygendemowidget.cpp
    oxygenframedemowidget.cpp
    oxygeninputdemowidget.cpp
    oxygenlistdemowidget.cpp
    oxygenmdidemowidget.cpp
    oxygensimulator.cpp
    oxygensliderdemowidget.cpp
    oxygentabdemowidget.cpp
    oxygentabwidget.cpp
    main.cpp)

set( oxygen_demo_FORMS
    ui/oxygenbenchmarkwidget.ui
    ui/oxygenbuttondemowidget.ui
    ui/oxygendemodialog.ui
    ui/oxygenframedemowidget.ui
    ui/oxygeninputdemowidget.ui
    ui/oxygenlistdemowidget.ui
    ui/oxygenmdidemowidget.ui
    ui/oxygensliderdemowidget.ui
    ui/oxygentabdemowidget.ui
)

qt5_wrap_ui(oxygen_demo_FORMS_HEADERS ${oxygen_demo_FORMS})

add_executable( oxygen-demo
    ${oxygen_demo_SOURCES}
    ${oxygen_demo_FORMS_HEADERS}
 )

target_link_libraries(oxygen-demo Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(oxygen-demo KF5::KCompletion KF5::KConfigGui KF5::KWidgetsAddons KF5::KI18n )

########### install files ###############
install(TARGETS oxygen-demo ${INSTALL_TARGETS_DEFAULT_ARGS} )

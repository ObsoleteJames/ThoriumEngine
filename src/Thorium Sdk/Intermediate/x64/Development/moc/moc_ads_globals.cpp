/****************************************************************************
** Meta object code from reading C++ file 'ads_globals.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/AdvancedDockWidgets/ads_globals.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ads_globals.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ads_t {
    QByteArrayData data[7];
    char stringdata0[82];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ads_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ads_t qt_meta_stringdata_ads = {
    {
QT_MOC_LITERAL(0, 0, 3), // "ads"
QT_MOC_LITERAL(1, 4, 15), // "SideBarLocation"
QT_MOC_LITERAL(2, 20, 10), // "SideBarTop"
QT_MOC_LITERAL(3, 31, 11), // "SideBarLeft"
QT_MOC_LITERAL(4, 43, 12), // "SideBarRight"
QT_MOC_LITERAL(5, 56, 13), // "SideBarBottom"
QT_MOC_LITERAL(6, 70, 11) // "SideBarNone"

    },
    "ads\0SideBarLocation\0SideBarTop\0"
    "SideBarLeft\0SideBarRight\0SideBarBottom\0"
    "SideBarNone"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ads[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       1,   14, // enums/sets
       0,    0, // constructors
       4,       // flags
       0,       // signalCount

 // enums: name, alias, flags, count, data
       1,    1, 0x0,    5,   19,

 // enum data: key, value
       2, uint(ads::SideBarTop),
       3, uint(ads::SideBarLeft),
       4, uint(ads::SideBarRight),
       5, uint(ads::SideBarBottom),
       6, uint(ads::SideBarNone),

       0        // eod
};

QT_INIT_METAOBJECT const QMetaObject ads::staticMetaObject = { {
    nullptr,
    qt_meta_stringdata_ads.data,
    qt_meta_data_ads,
    nullptr,
    nullptr,
    nullptr
} };

QT_WARNING_POP
QT_END_MOC_NAMESPACE

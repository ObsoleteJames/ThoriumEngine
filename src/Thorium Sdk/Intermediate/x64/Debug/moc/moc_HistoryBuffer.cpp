/****************************************************************************
** Meta object code from reading C++ file 'HistoryBuffer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/HistoryBuffer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HistoryBuffer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CHistoryBuffer_t {
    QByteArrayData data[7];
    char stringdata0[59];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CHistoryBuffer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CHistoryBuffer_t qt_meta_stringdata_CHistoryBuffer = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CHistoryBuffer"
QT_MOC_LITERAL(1, 15, 12), // "onEventAdded"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 6), // "onUndo"
QT_MOC_LITERAL(4, 36, 8), // "SizeType"
QT_MOC_LITERAL(5, 45, 6), // "cursor"
QT_MOC_LITERAL(6, 52, 6) // "onRedo"

    },
    "CHistoryBuffer\0onEventAdded\0\0onUndo\0"
    "SizeType\0cursor\0onRedo"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CHistoryBuffer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    1,   30,    2, 0x06 /* Public */,
       6,    1,   33,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,

       0        // eod
};

void CHistoryBuffer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CHistoryBuffer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onEventAdded(); break;
        case 1: _t->onUndo((*reinterpret_cast< SizeType(*)>(_a[1]))); break;
        case 2: _t->onRedo((*reinterpret_cast< SizeType(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CHistoryBuffer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHistoryBuffer::onEventAdded)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CHistoryBuffer::*)(SizeType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHistoryBuffer::onUndo)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CHistoryBuffer::*)(SizeType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHistoryBuffer::onRedo)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CHistoryBuffer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CHistoryBuffer.data,
    qt_meta_data_CHistoryBuffer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CHistoryBuffer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CHistoryBuffer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CHistoryBuffer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CHistoryBuffer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void CHistoryBuffer::onEventAdded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CHistoryBuffer::onUndo(SizeType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CHistoryBuffer::onRedo(SizeType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

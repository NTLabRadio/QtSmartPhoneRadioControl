#ifndef TYPEDEF_H
#define TYPEDEF_H

#include "LittleBigEndian.h"

typedef BigEndian<QByteArray> QByteArrayBE;
typedef LittleEndian<QByteArray> QByteArrayLE;

typedef LittleEndian<quint16> u16le;
typedef LittleEndian<quint32> u32le;
typedef LittleEndian<quint64> u64le;

typedef BigEndian<quint16> u16be;
typedef BigEndian<quint32> u32be;
typedef BigEndian<quint64> u64be;

#endif // TYPEDEF_H

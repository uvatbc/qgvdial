#ifndef __SHARED_DATA_TYPES__
#define __SHARED_DATA_TYPES__

#include <QtCore>
#include <QtDBus>

////////////////////////////////////////////////////////////////////////////////
// a{sv} = QVariantMap

////////////////////////////////////////////////////////////////////////////////
// a(susv)
struct Struct_susv {
    QString     s1;
    uint        u;
    QString     s2;
    QDBusVariant    v;
};
Q_DECLARE_METATYPE(Struct_susv)

typedef QList<Struct_susv> Qt_Type_a_susv;
Q_DECLARE_METATYPE(Qt_Type_a_susv)

////////////////////////////////////////////////////////////////////////////////
// au
typedef QList<uint> Qt_Type_au;
Q_DECLARE_METATYPE(Qt_Type_au)

////////////////////////////////////////////////////////////////////////////////
// a{us}
typedef QMap<unsigned, QString> Qt_Type_dict_us;
Q_DECLARE_METATYPE(Qt_Type_dict_us)

////////////////////////////////////////////////////////////////////////////////
// a(su)
struct Struct_su {
    QString     s;
    uint        u;
};
Q_DECLARE_METATYPE(Struct_su)

typedef QList<Struct_su> Qt_Type_a_su;
Q_DECLARE_METATYPE(Qt_Type_a_su)

////////////////////////////////////////////////////////////////////////////////
// a(us)
struct Struct_us {
    uint        u;
    QString     s;
};
Q_DECLARE_METATYPE(Struct_us)

typedef QList<Struct_us> Qt_Type_a_us;
Q_DECLARE_METATYPE(Qt_Type_a_us)

////////////////////////////////////////////////////////////////////////////////
// a(usuu)
struct Struct_usuu {
    unsigned u1;
    QString  s;
    unsigned u2;
    unsigned u3;
};
Q_DECLARE_METATYPE(Struct_usuu)

typedef QList<Struct_usuu> Qt_Type_a_usuu;
Q_DECLARE_METATYPE(Qt_Type_a_usuu)

////////////////////////////////////////////////////////////////////////////////
// a(usuuuu)
struct Struct_usuuuu {
    unsigned u1;
    QString  s;
    unsigned u2;
    unsigned u3;
    unsigned u4;
    unsigned u5;
};
Q_DECLARE_METATYPE(Struct_usuuuu)

typedef QList<Struct_usuuuu> Qt_Type_a_usuuuu;
Q_DECLARE_METATYPE(Qt_Type_a_usuuuu)

////////////////////////////////////////////////////////////////////////////////
// a{ua{sv}}
typedef QMap<unsigned, QVariantMap> Qt_Type_dict_u_dict_sv;
Q_DECLARE_METATYPE(Qt_Type_dict_u_dict_sv)

////////////////////////////////////////////////////////////////////////////////
// a{u(uus)}
struct Struct_uus {
    unsigned u1;
    unsigned u2;
    QString s;
};
Q_DECLARE_METATYPE(Struct_uus)

typedef QMap<unsigned, Struct_uus> Qt_Type_dict_u_uus;
Q_DECLARE_METATYPE(Qt_Type_dict_u_uus)

////////////////////////////////////////////////////////////////////////////////
// a(osuu)
struct Struct_osuu {
    QString o;  // Object path
    QString s;
    unsigned u1;
    unsigned u2;
};
Q_DECLARE_METATYPE(Struct_osuu)

typedef QList<Struct_osuu> Qt_Type_a_osuu;
Q_DECLARE_METATYPE(Qt_Type_a_osuu)

////////////////////////////////////////////////////////////////////////////////
// a{u(ua{sa{sv}})}: Lets do it in steps:
// 1. a{sa{sv}}
typedef QMap<QString, QVariantMap> Qt_Type_dict_s_dict_sv;
Q_DECLARE_METATYPE(Qt_Type_dict_s_dict_sv)

// 2. (u$1) = (ua{sa{sv}})
struct Struct_u_dict_s_dict_sv {
    unsigned u;
    Qt_Type_dict_s_dict_sv dict_s_dict_sv;
};
Q_DECLARE_METATYPE(Struct_u_dict_s_dict_sv)

// 3. a{u$2} = final step = a{u(ua{sa{sv}})}
typedef QMap<unsigned, Struct_u_dict_s_dict_sv> Qt_Type_dict_u_u_dict_s_dict_sv;
Q_DECLARE_METATYPE(Qt_Type_dict_u_u_dict_s_dict_sv)

////////////////////////////////////////////////////////////////////////////////
// a{s(ubba{ss})}: Lets do it in steps:
// 1: a{ss}
typedef QMap<QString, QString> Qt_Type_dict_ss;
Q_DECLARE_METATYPE(Qt_Type_dict_ss)

// 2: (ubb$1) = (ubba{ss})
struct Struct_ubb_dict_ss {
    uint        u;
    bool        b1;
    bool        b2;
    Qt_Type_dict_ss dict_ss;
};
Q_DECLARE_METATYPE(Struct_ubb_dict_ss)

// 3: a{s$2} = final step = a{s(ubba{ss})}
typedef QMap<QString, Struct_ubb_dict_ss> Qt_Type_dict_s_ubb_dict_ss;
Q_DECLARE_METATYPE(Qt_Type_dict_s_ubb_dict_ss)

////////////////////////////////////////////////////////////////////////////////
// a(oa{sv}): Lets do it in steps:
// 1. (oa{sv})
struct Struct_o_dict_sv {
    QString o;
    QVariantMap vmap;
};
Q_DECLARE_METATYPE(Struct_o_dict_sv)

// 2: a$1
typedef QList<Struct_o_dict_sv> Qt_Type_a_o_dict_sv;
Q_DECLARE_METATYPE(Qt_Type_a_o_dict_sv)

////////////////////////////////////////////////////////////////////////////////
// a{u(uss)}
struct Struct_uss {
    unsigned u;
    QString s1;
    QString s2;
};
Q_DECLARE_METATYPE(Struct_uss)

typedef QMap<unsigned, Struct_uss> Qt_Type_dict_u_uss;
Q_DECLARE_METATYPE(Qt_Type_dict_u_uss)

////////////////////////////////////////////////////////////////////////////////
// a(a{sv}as)
struct Struct_dict_sv_as {
    QVariantMap sv;
    QStringList as;
};
Q_DECLARE_METATYPE(Struct_dict_sv_as)

typedef QList<Struct_dict_sv_as> Qt_Type_a_dict_sv_as;
Q_DECLARE_METATYPE(Qt_Type_a_dict_sv_as)

#include "QGVConnection.h"
#include "QGVConnectionManager.h"
    
void registerDBusTypes();
const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_susv &param);
QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_susv &param);

#endif//__SHARED_DATA_TYPES__

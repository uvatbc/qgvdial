/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "shared_data_types.h"

void
registerDBusTypes()
{
    qDBusRegisterMetaType<Struct_susv>();
    qDBusRegisterMetaType<Qt_Type_a_susv>();
    qDBusRegisterMetaType<Qt_Type_au>();
    qDBusRegisterMetaType<Qt_Type_dict_us>();
    qDBusRegisterMetaType<Struct_su>();
    qDBusRegisterMetaType<Qt_Type_a_su>();
    qDBusRegisterMetaType<Struct_us>();
    qDBusRegisterMetaType<Qt_Type_a_us>();
    qDBusRegisterMetaType<Struct_usuu>();
    qDBusRegisterMetaType<Qt_Type_a_usuu>();
    qDBusRegisterMetaType<Struct_usuuuu>();
    qDBusRegisterMetaType<Qt_Type_a_usuuuu>();
    qDBusRegisterMetaType<Qt_Type_dict_u_dict_sv>();
    qDBusRegisterMetaType<Struct_uus>();
    qDBusRegisterMetaType<Qt_Type_dict_u_uus>();
    qDBusRegisterMetaType<Struct_osuu>();
    qDBusRegisterMetaType<Qt_Type_a_osuu>();
    qDBusRegisterMetaType<Qt_Type_dict_s_dict_sv>();
    qDBusRegisterMetaType<Struct_u_dict_s_dict_sv>();
    qDBusRegisterMetaType<Qt_Type_dict_u_u_dict_s_dict_sv>();
    qDBusRegisterMetaType<Qt_Type_dict_ss>();
    qDBusRegisterMetaType<Struct_ubb_dict_ss>();
    qDBusRegisterMetaType<Qt_Type_dict_s_ubb_dict_ss>();
    qDBusRegisterMetaType<Struct_o_dict_sv>();
    qDBusRegisterMetaType<Qt_Type_a_o_dict_sv>();
    qDBusRegisterMetaType<Struct_uss>();
    qDBusRegisterMetaType<Qt_Type_dict_u_uss>();
    qDBusRegisterMetaType<Struct_dict_sv_as>();
    qDBusRegisterMetaType<Qt_Type_a_dict_sv_as>();
    qDBusRegisterMetaType<Qt_Type_a_dict_sv>();
    qDBusRegisterMetaType<Qt_Type_a_a_dict_sv>();
    qDBusRegisterMetaType<Qt_Type_dict_uv>();
    qDBusRegisterMetaType<Struct_uuuuus>();
    qDBusRegisterMetaType<Qt_Type_a_uuuuus>();
}//registerDBusTypes

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_susv &param)
{
    // I don't know how this works or why without it there are compiler errors.
    // I just copied it from the Qt documentation for QDBusArgument
    argument.beginStructure();
    argument >> param.s1 >> param.u >> param.s2 >> param.v;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_susv &param)
{
    argument.beginStructure();
    argument << param.s1 << param.u << param.s2 << param.v;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_su &param)
{
    argument.beginStructure();
    argument >> param.s >> param.u;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_su &param)
{
    argument.beginStructure();
    argument << param.s << param.u;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_us &param)
{
    argument.beginStructure();
    argument >> param.u >> param.s;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_us &param)
{
    argument.beginStructure();
    argument << param.u << param.s;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_usuu &param)
{
    argument.beginStructure();
    argument >> param.u1 >> param.s >> param.u2 >> param.u3;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_usuu &param)
{
    argument.beginStructure();
    argument << param.u1 << param.s << param.u2 << param.u3;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_usuuuu &param)
{
    argument.beginStructure();
    argument >> param.u1 >> param.s >> param.u2 >> param.u3 >> param.u4 >> param.u5;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_usuuuu &param)
{
    argument.beginStructure();
    argument << param.u1 << param.s << param.u2 << param.u3 << param.u4 << param.u5;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_uus &param)
{
    argument.beginStructure();
    argument >> param.u1 >> param.u2 >> param.s;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_uus &param)
{
    argument.beginStructure();
    argument << param.u1 << param.u2 << param.s;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_osuu &param)
{
    argument.beginStructure();
    argument >> param.o >> param.s >> param.u1 >> param.u2;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_osuu &param)
{
    argument.beginStructure();
    argument << param.o << param.s << param.u1 << param.u2;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_u_dict_s_dict_sv &param)
{
    argument.beginStructure();
    argument >> param.u >> param.dict_s_dict_sv;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_u_dict_s_dict_sv &param)
{
    argument.beginStructure();
    argument << param.u << param.dict_s_dict_sv;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_ubb_dict_ss &param)
{
    argument.beginStructure();
    argument >> param.u >> param.b1 >> param.b2 >> param.dict_ss;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_ubb_dict_ss &param)
{
    argument.beginStructure();
    argument << param.u << param.b1 << param.b2 << param.dict_ss;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_o_dict_sv &param)
{
    argument.beginStructure();
    argument >> param.o >> param.vmap;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_o_dict_sv &param)
{
    argument.beginStructure();
    argument << param.o << param.vmap;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_uss &param)
{
    argument.beginStructure();
    argument >> param.u >> param.s1 >> param.s2;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_uss &param)
{
    argument.beginStructure();
    argument << param.u << param.s1 << param.s2;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_dict_sv_as &param)
{
    argument.beginStructure();
    argument >> param.sv >> param.as;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_dict_sv_as &param)
{
    argument.beginStructure();
    argument << param.sv << param.as;
    argument.endStructure();
    return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, Struct_uuuuus &param)
{
    argument.beginStructure();
    argument >> param.u1 >> param.u2 >> param.u3 >> param.u4 >> param.u5 >> param.s;
    argument.endStructure();
    return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const Struct_uuuuus &param)
{
    argument.beginStructure();
    argument << param.u1 << param.u2 << param.u3 << param.u4 << param.u5 << param.s;
    argument.endStructure();
    return argument;
}

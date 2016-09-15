#include "shared_data_types.h"

void
registerDBusTypes()
{
    qDBusRegisterMetaType<Struct_susv>();
    qDBusRegisterMetaType<Qt_Type_a_susv>();
/*
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
*/
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


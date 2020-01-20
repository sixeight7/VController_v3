#include "globaldevices.h"

GP10_class My_GP10 = GP10_class(GP10); // Deze wil niet compileren - linker error!!!
GR55_class My_GR55 = GR55_class(GR55);
VG99_class My_VG99 = VG99_class(VG99);
ZG3_class My_ZG3 = ZG3_class(ZG3);
ZMS70_class My_ZMS70 = ZMS70_class(ZMS70);
M13_class My_M13 = M13_class(M13);
HLX_class My_HLX = HLX_class(HLX);
AXEFX_class My_AXEFX = AXEFX_class(AXEFX);
KTN_class My_KTN = KTN_class(KTN);
KPA_class My_KPA = KPA_class(KPA);
SVL_class My_SVL = SVL_class(SVL);

Device_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS70, &My_M13, &My_HLX, &My_AXEFX, &My_KTN, &My_KPA, &My_SVL};

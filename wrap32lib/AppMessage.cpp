#include "AppMessage.h"

UINT GetUniqueAppMessage() { static UINT uAWMNext = WM_APP;  return uAWMNext++; }


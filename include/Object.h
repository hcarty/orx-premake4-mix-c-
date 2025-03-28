/**
 * @file Object.h
 * @date 25-Mar-2025
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "orx-premake4-mix-c++.h"

/** Object Class
 */
class Object : public ScrollObject
{
public:


protected:

                void            OnCreate();
                void            OnDelete();
                void            Update(const orxCLOCK_INFO &_rstInfo);


private:
};

#endif // __OBJECT_H__

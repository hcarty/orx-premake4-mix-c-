/**
 * @file orx-premake4-mix-c++.h
 * @date 25-Mar-2025
 */

#ifndef __orx_premake4_mix_c___H__
#define __orx_premake4_mix_c___H__

#include "Scroll/Scroll.h"

/** Game Class
 */
class orx_premake4_mix_c__ : public Scroll<orx_premake4_mix_c__>
{
public:


private:

                orxSTATUS       Bootstrap() const;

                void            Update(const orxCLOCK_INFO &_rstInfo);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();


private:
};

#endif // __orx_premake4_mix_c___H__

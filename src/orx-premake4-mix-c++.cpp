/**
 * @file orx-premake4-mix-c++.cpp
 * @date 25-Mar-2025
 */

#define __SCROLL_IMPL__
#include "orx-premake4-mix-c++.h"
#undef __SCROLL_IMPL__

#include "Object.h"
#include "orxExtensions.h"

#ifdef __orxMSVC__

/* Requesting high performance dedicated GPU on hybrid laptops */
__declspec(dllexport) unsigned long NvOptimusEnablement        = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // __orxMSVC__

/** Update function, it has been registered to be called every tick of the core clock
 */
void orx_premake4_mix_c__::Update(const orxCLOCK_INFO &_rstClockInfo)
{
  // Show Dear ImGui's demo and stats windows
  ImGui::ShowDemoWindow();
  ImGui::ShowMetricsWindow();

  // Call Python update
  
  orxPy_Update(&_rstClockInfo, orxNULL);

}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orx_premake4_mix_c__::Init()
{
  orxSTATUS eStatus = orxSTATUS_SUCCESS;

  // Init extensions
  InitExtensions();


  // Push [Main] as the default section
  orxConfig_PushSection("Main");

  // Create the viewports
  for(orxS32 i = 0, iCount = orxConfig_GetListCount("ViewportList"); i < iCount; i++)
  {
    orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
  }
  // Call Python init function
  eStatus = orxPy_Init();


  // Done!
  return eStatus;

}

/** Run function, it should not contain any game logic
 */
orxSTATUS orx_premake4_mix_c__::Run()
{
  // Return orxSTATUS_FAILURE to instruct orx to quit
  return orxSTATUS_SUCCESS;
}

/** Exit function, it is called before exiting from orx
 */
void orx_premake4_mix_c__::Exit()
{
  // Exit from extensions
  ExitExtensions();

  // Let orx clean all our mess automatically. :)
}

/** BindObjects function, ScrollObject-derived classes are bound to config sections here
 */
void orx_premake4_mix_c__::BindObjects()
{
  // Bind the Object class
  BindObject(Object);
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS orx_premake4_mix_c__::Bootstrap() const
{
  // Bootstrap extensions
  BootstrapExtensions();

  // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
  return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
  // Execute our game
  orx_premake4_mix_c__::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}

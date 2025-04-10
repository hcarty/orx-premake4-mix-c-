//! Includes

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef _orxPY_H_
#define _orxPY_H_

#include "orx.h"

#include "pocketpy.h"

  //! Defines

#define orxPY_KZ_RESOURCE "Python"

#define orxPY_KZ_CONFIG_SECTION "Python"
#define orxPY_KZ_CONFIG_MAIN "Main"

#define orxPY_KZ_COMMAND_EXEC "Python.Exec"

#define orxPY_KZ_DEFAULT_INIT "on_init"
#define orxPY_KZ_DEFAULT_UPDATE "on_update"
#define orxPY_KZ_DEFAULT_EXIT "on_exit"

#define orxPY_KZ_DEFAULT_OBJECT_CREATE_EVENT_HANDLER "on_create"
#define orxPY_KZ_DEFAULT_OBJECT_DELETE_EVENT_HANDLER "on_delete"
#define orxPY_KZ_DEFAULT_PHYSICS_COLLIDE_EVENT_HANDLER "on_collide"
#define orxPY_KZ_DEFAULT_PHYSICS_SEPARATE_EVENT_HANDLER "on_separate"
#define orxPY_KZ_DEFAULT_SHADER_PARAM_EVENT_HANDLER "on_shader_param"

  //! Custom Python types

  static py_Type tp_orxguid;
  static py_Type tp_orxvector;
  static py_Type tp_orxobject;
  static py_Type tp_orxclock;

  //! Callbacks

  orxSTATUS orxPy_Init();
  void orxPy_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext);

#ifdef orxPY_IMPL

  //! Variables / Structures

  typedef struct __orxPY_t
  {
    orxBOOL bInitialized;    // Initialized state
    py_GlobalRef pyMod;      // Module containing callbacks
    py_Name pyInit;          // Engine init callback
    py_Name pyUpdate;        // Engine update callback
    py_Name pyExit;          // Engine exit callback
    py_Name pyOnCreate;      // Object creation callback
    py_Name pyOnDelete;      // Object deletion callback
    py_Name pyOnCollide;     // Object collision callback
    py_Name pyOnSeparate;    // Object separation callback
    py_Name pyOnShaderParam; // Shader parameter event
  } orxPY;

  static orxPY sstPy;

  //! Code

#define orxPy_GetCallback(field) py_getdict(sstPy.pyMod, field)

#define orxPY_KZ_LOG_TAG orxANSI_KZ_COLOR_FG_YELLOW "[PYTHON] " orxANSI_KZ_COLOR_RESET

  void orxPy_Print(const orxSTRING zData)
  {
    orxLOG("%s", zData);
  }

  void orxPy_LogException()
  {
    orxSTRING zMessage = py_formatexc();
    orxLOG("%s", zMessage);
    orxMemory_Free(zMessage);
  }

  orxCHAR *orxPy_ReadSource(const orxSTRING zPath)
  {
    orxCHAR *pBuffer = orxNULL;

    // Load map resource
    const orxCHAR *zResourceLocation = orxResource_Locate(orxPY_KZ_RESOURCE, zPath);
    if (zResourceLocation != orxNULL)
    {
      orxLOG(orxPY_KZ_LOG_TAG "Loading %s from %s", zPath, zResourceLocation);
      orxHANDLE hResource = orxResource_Open(zResourceLocation, orxFALSE);
      orxASSERT(hResource != orxHANDLE_UNDEFINED);

      orxS64 s64Size = orxResource_GetSize(hResource) + 1;
      pBuffer = (orxCHAR *)orxMemory_Allocate(s64Size, orxMEMORY_TYPE_MAIN);
      orxASSERT(pBuffer != orxNULL);

      orxS64 s64Read = orxResource_Read(hResource, s64Size, pBuffer, orxNULL, orxNULL);
      orxResource_Close(hResource);
      orxASSERT(s64Read == s64Size - 1);
      pBuffer[s64Read] = orxCHAR_NULL;
    }

    return pBuffer;
  }

  orxSTATUS orxPy_Call(py_ItemRef pyCallable)
  {
    orxSTATUS eResult = orxSTATUS_FAILURE;

    if (pyCallable != orxNULL)
    {
      if (py_call(pyCallable, 0, orxNULL))
      {
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxPy_LogException();
      }
    }

    return eResult;
  }

  orxSTATUS orxPy_Call1(py_ItemRef pyCallable, py_Ref pyArg)
  {
    orxSTATUS eResult = orxSTATUS_FAILURE;

    if (pyCallable != orxNULL)
    {
      py_push(pyCallable);
      py_pushnil();
      py_push(pyArg);
      if (py_vectorcall(1, 0))
      {
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxPy_LogException();
      }
    }

    return eResult;
  }

  orxSTATUS orxPy_CallN(py_Ref pyCallable, orxS32 s32Argc, py_Ref pyArgv)
  {
    orxSTATUS eResult = orxSTATUS_FAILURE;

    if (pyCallable != orxNULL)
    {
      if (py_call(pyCallable, s32Argc, pyArgv))
      {
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxPy_LogException();
      }
    }

    return eResult;
  }

  void orxPy_CommandPyExec(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
  {
    py_StackRef pyStackPos = py_peek(0);

    /* Execute source */
    bool bResult = py_exec(_astArgList[0].zValue, "<command>", SINGLE_MODE, orxNULL);

    if (!bResult)
    {
      if (_u32ArgNumber == 1 || _u32ArgNumber > 1 && _astArgList[1].bValue == orxTRUE)
      {
        orxPy_LogException();
      }
      py_clearexc(pyStackPos);
    }

    /* Set result */
    _pstResult->bValue = bResult;

    /* Done! */
    return;
  }

  void orxPy_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
  {
    orxPROFILER_PUSH_MARKER(__FUNCTION__);
    if (sstPy.pyUpdate != 0)
    {
      py_Ref pyArg = py_r0();
      py_newfloat(pyArg, _pstClockInfo->fDT);
      orxPy_Call1(orxPy_GetCallback(sstPy.pyUpdate), pyArg);
    }
    orxPROFILER_POP_MARKER();
  }

// Binding naming
#define BINDING_NAME(PREFIX, NAME) orxPyBind_##PREFIX##_##NAME
#define BIND(PREFIX, NAME) bool BINDING_NAME(PREFIX, NAME)(int argc, py_Ref argv)

// Binding argument handling
#define ARG_VALUE(ret_type, py_type, name, index) \
  PY_CHECK_ARG_TYPE(index, tp_##py_type);         \
  ret_type name = py_to##py_type(py_arg(index))
#define ARG_VALUE_OR_NONE(ret_type, py_type, name, index) \
  ret_type name;                                          \
  do                                                      \
  {                                                       \
    if (py_isnone(py_arg(index)))                         \
    {                                                     \
      name = orxNULL;                                     \
    }                                                     \
    else                                                  \
    {                                                     \
      PY_CHECK_ARG_TYPE(index, tp_##py_type);             \
      name = py_to##py_type(py_arg(index));               \
    }                                                     \
  } while (0)
#define ARG_STR(name, index) ARG_VALUE(const orxSTRING, str, name, index)
#define ARG_STR_OR_NONE(name, index) ARG_VALUE_OR_NONE(const orxSTRING, str, name, index)
#define ARG_FLOAT(name, index)                 \
  orxFLOAT name;                               \
  do                                           \
  {                                            \
    if (!py_castfloat32(py_arg(index), &name)) \
      return false;                            \
  } while (0)
#define ARG_INT(name, index) ARG_VALUE(orxS64, int, name, index)
#define ARG_BOOL(name, index) ARG_VALUE(orxBOOL, bool, name, index)
#define ARG_PTR(type, name, index) type *name = (type *)py_touserdata(py_arg(index))
#define ARG_GUID(name, index)             \
  orxU64 name;                            \
  do                                      \
  {                                       \
    PY_CHECK_ARG_TYPE(index, tp_orxguid); \
    ARG_PTR(orxU64, p##name, index);      \
    name = *p##name;                      \
  } while (0)
#define ARG_OBJECT(name, index)             \
  orxOBJECT *name;                          \
  do                                        \
  {                                         \
    PY_CHECK_ARG_TYPE(index, tp_orxobject); \
    ARG_PTR(orxOBJECT *, p##name, index);   \
    name = *p##name;                        \
  } while (0)
#define OBJECT ARG_OBJECT(pstObject, 0)
#define ARG_VECTOR(name, index)                \
  orxVECTOR *name;                             \
  do                                           \
  {                                            \
    PY_CHECK_ARG_TYPE(index, tp_orxvector);    \
    ARG_PTR(orxVECTOR, pVector##index, index); \
    name = pVector##index;                     \
  } while (0)
#define ARG_OBJECT_OR_NONE(name, index)       \
  orxOBJECT *name;                            \
  do                                          \
  {                                           \
    if (py_isnone(py_arg(index)))             \
    {                                         \
      name = orxNULL;                         \
    }                                         \
    else                                      \
    {                                         \
      PY_CHECK_ARG_TYPE(index, tp_orxobject); \
      ARG_PTR(orxOBJECT *, p##name, index);   \
      name = *p##name;                        \
    }                                         \
  } while (0)
#define ARG_CLOCK(name, index)             \
  orxCLOCK *name;                          \
  do                                       \
  {                                        \
    PY_CHECK_ARG_TYPE(index, tp_orxclock); \
    ARG_PTR(orxCLOCK *, p##name, index);   \
    name = *p##name;                       \
  } while (0)

// Binding return value handling
#define RETURN_VALUE(CONV, RET) \
  do                            \
  {                             \
    CONV(py_retval(), RET);     \
    return true;                \
  } while (0)
#define RETURN_STR(RET) RETURN_VALUE(py_newstr, RET)
#define RETURN_FLOAT(RET) RETURN_VALUE(py_newfloat, RET)
#define RETURN_INT(RET) RETURN_VALUE(py_newint, RET)
#define RETURN_BOOL(RET) RETURN_VALUE(py_newbool, RET)
#define RETURN_GUID(RET) RETURN_VALUE(py_neworxguid, RET)
#define RETURN_VECTOR(RET) RETURN_VALUE(py_neworxvector, RET)
#define RETURN_CLOCK(RET) RETURN_VALUE(py_neworxclock, RET)
#define RETURN_NONE          \
  do                         \
  {                          \
    py_newnone(py_retval()); \
    return true;             \
  } while (0)
#define RETURN_OBJECT_OR_NONE(OBJ)       \
  do                                     \
  {                                      \
    if (OBJ == orxNULL)                  \
    {                                    \
      RETURN_NONE;                       \
    }                                    \
    else                                 \
    {                                    \
      py_neworxobject(py_retval(), OBJ); \
      return true;                       \
    }                                    \
  } while (0)

  // Function wrappers

  // Helpers

  void py_neworxguid(py_OutRef pyOut, orxU64 _u64GUID)
  {
    orxU64 *pu64GUID = (orxU64 *)py_newobject(pyOut, tp_orxguid, 0, sizeof(orxU64));
    *pu64GUID = _u64GUID;
  }

  void py_neworxobject(py_OutRef pyOut, orxOBJECT *_pstObject)
  {
    orxOBJECT **ppstObject = (orxOBJECT **)py_newobject(pyOut, tp_orxobject, 0, sizeof(orxOBJECT *));
    *ppstObject = _pstObject;
  }

  void py_neworxvector(py_OutRef pyOut, orxVECTOR *_pvVector)
  {
    orxVECTOR *pvVector = (orxVECTOR *)py_newobject(pyOut, tp_orxvector, 0, sizeof(orxVECTOR));
    orxVector_Copy(pvVector, _pvVector);
  }

  void py_neworxclock(py_OutRef pyOut, orxCLOCK *_pstClock)
  {
    orxCLOCK **ppstClock = (orxCLOCK **)py_newobject(pyOut, tp_orxclock, 0, sizeof(orxCLOCK *));
    *ppstClock = _pstClock;
  }

  // Object functions

  BIND(Object, __new__)
  {
    if (argc != 1 && argc != 2)
    {
      return TypeError("expected %d arguments, got %d", 1, argc);
    }

    // Are we being called as an object constructor from a class?
    bool bIsConstructor = argc == 2;
    orxU16 u16ArgIndex = argc - 1;

    if (py_istype(py_arg(u16ArgIndex), tp_str))
    {
      // Create from a config section name
      ARG_STR(zName, u16ArgIndex);
      orxOBJECT *pstObject = orxObject_CreateFromConfig(zName);
      if (pstObject == orxNULL && bIsConstructor)
      {
        // Object constructor, so raise an exception if an object cannot be created
        return ValueError("Unable to create object from config section %s", zName);
      }
      // Object successfully created or this is a function call. Return result either way.
      RETURN_OBJECT_OR_NONE(pstObject);
    }
    else if (py_istype(py_arg(u16ArgIndex), tp_orxguid))
    {
      // Retrieve from a GUID
      ARG_GUID(u64GUID, u16ArgIndex);
      orxOBJECT *pstObject = orxOBJECT(orxStructure_Get(u64GUID));
      if (pstObject == orxNULL && bIsConstructor)
      {
        // Object constructor, so raise an exception if an object cannot be created
        return ValueError("Unable to create object from GUID 0x%016llX", u64GUID);
      }
      // Object successfully created or this is a function call. Return result either way.
      RETURN_OBJECT_OR_NONE(pstObject);
    }
    else
    {
      return ValueError("An orx object can only be constructed from a config section name or GUID");
    }
    // Should be impossible to reach this point
    return RuntimeError("Impossible object creation state reached");
  }

  BIND(Object, __eq__)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_OBJECT(pstComp, 1);
    RETURN_BOOL(pstObject == pstComp);
  }

  BIND(Object, __hash__)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_INT((py_i64)pstObject);
  }

  BIND(Object, __str__)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxCHAR acInstanceName[20];
    orxString_NPrint(acInstanceName, sizeof(acInstanceName), "0x%016llX", orxStructure_GetGUID(pstObject));
    RETURN_STR(acInstanceName);
  }

  BIND(Object, delete)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_Delete(pstObject);
    RETURN_NONE;
  }

  BIND(Object, get_guid)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_GUID(orxStructure_GetGUID(orxSTRUCTURE(pstObject)));
  }

  BIND(Object, enable)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_BOOL(bState, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_EnableRecursive(pstObject, bState);
    }
    else
    {
      orxObject_Enable(pstObject, bState);
    }
    RETURN_NONE;
  }

  BIND(Object, is_enabled)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_BOOL(orxObject_IsEnabled(pstObject));
  }

  BIND(Object, pause)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_BOOL(bState, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_PauseRecursive(pstObject, bState);
    }
    else
    {
      orxObject_Pause(pstObject, bState);
    }
    RETURN_NONE;
  }

  BIND(Object, is_paused)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_BOOL(orxObject_IsPaused(pstObject));
  }

  BIND(Object, set_owner)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_OBJECT(pstOwner, 1);
    orxObject_SetOwner(pstObject, (void *)pstOwner);
    RETURN_NONE;
  }

  BIND(Object, get_owner)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxOBJECT *pstOwner = orxOBJECT(orxObject_GetOwner(pstObject));
    RETURN_OBJECT_OR_NONE(pstOwner);
  }

  BIND(Object, find_owned_child)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zPath, 1);
    orxOBJECT *pstChild = orxObject_FindOwnedChild(pstObject, zPath);
    RETURN_OBJECT_OR_NONE(pstChild);
  }

  BIND(Object, set_clock)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_CLOCK(pstClock, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetClockRecursive(pstObject, pstClock);
    }
    else
    {
      orxObject_SetClock(pstObject, pstClock);
    }
    RETURN_NONE;
  }

  BIND(Object, get_clock)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxCLOCK *pstClock = orxObject_GetClock(pstObject);
    if (pstClock == orxNULL)
    {
      RETURN_NONE;
    }
    RETURN_CLOCK(pstClock);
  }

  BIND(Object, set_flip)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_BOOL(bFlipX, 1);
    ARG_BOOL(bFlipY, 2);
    orxObject_SetFlip(pstObject, bFlipX, bFlipY);
    RETURN_NONE;
  }

  BIND(Object, get_flip)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxBOOL bFlipX, bFlipY;
    orxObject_GetFlip(pstObject, &bFlipX, &bFlipY);
    py_newtuple(py_retval(), 2);
    py_newbool(py_tuple_getitem(py_retval(), 0), bFlipX);
    py_newbool(py_tuple_getitem(py_retval(), 1), bFlipY);
    return true;
  }

  BIND(Object, set_position)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(pvPosition, 1);
    ARG_BOOL(bWorld, 2);
    if (bWorld)
    {
      orxObject_SetWorldPosition(pstObject, pvPosition);
    }
    else
    {
      orxObject_SetPosition(pstObject, pvPosition);
    }
    RETURN_NONE;
  }

  BIND(Object, get_position)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bWorld, 1);
    orxVECTOR vPosition;
    if (bWorld)
    {
      orxObject_GetWorldPosition(pstObject, &vPosition);
    }
    else
    {
      orxObject_GetPosition(pstObject, &vPosition);
    }
    RETURN_VECTOR(&vPosition);
  }

  BIND(Object, set_rotation)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_FLOAT(fRotation, 1);
    ARG_BOOL(bWorld, 2);
    if (bWorld)
    {
      orxObject_SetWorldRotation(pstObject, fRotation);
    }
    else
    {
      orxObject_SetRotation(pstObject, fRotation);
    }
    RETURN_NONE;
  }

  BIND(Object, get_rotation)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bWorld, 1);
    orxFLOAT fRotation;
    if (bWorld)
    {
      fRotation = orxObject_GetWorldRotation(pstObject);
    }
    else
    {
      fRotation = orxObject_GetRotation(pstObject);
    }
    RETURN_FLOAT(fRotation);
  }

  BIND(Object, set_scale)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(pvScale, 1);
    ARG_BOOL(bWorld, 2);
    if (bWorld)
    {
      orxObject_SetWorldScale(pstObject, pvScale);
    }
    else
    {
      orxObject_SetScale(pstObject, pvScale);
    }
    RETURN_NONE;
  }

  BIND(Object, get_scale)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bWorld, 1);
    orxVECTOR vScale;
    if (bWorld)
    {
      orxObject_GetWorldScale(pstObject, &vScale);
    }
    else
    {
      orxObject_GetScale(pstObject, &vScale);
    }
    RETURN_VECTOR(&vScale);
  }

  BIND(Object, set_parent)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_OBJECT_OR_NONE(pstParent, 1);
    orxObject_SetParent(pstObject, pstParent);
    RETURN_NONE;
  }

  BIND(Object, get_parent)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxOBJECT *pstParent = orxOBJECT(orxObject_GetParent(pstObject));
    RETURN_OBJECT_OR_NONE(pstParent);
  }

  BIND(Object, find_child)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zPath, 1);
    orxOBJECT *pstChild = orxObject_FindChild(pstObject, zPath);
    RETURN_OBJECT_OR_NONE(pstChild);
  }

  BIND(Object, attach)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_PTR(orxOBJECT, pstParent, 1);
    orxObject_Attach(pstObject, (void *)pstParent);
    RETURN_NONE;
  }

  BIND(Object, detach)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_Detach(pstObject);
    RETURN_NONE;
  }

  BIND(Object, log_parents)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_LogParents(pstObject);
    RETURN_NONE;
  }

  BIND(Object, set_anim_frequency)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_FLOAT(fFrequency, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetAnimFrequencyRecursive(pstObject, fFrequency);
    }
    else
    {
      orxObject_SetAnimFrequency(pstObject, fFrequency);
    }
    RETURN_NONE;
  }

  BIND(Object, get_anim_frequency)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetAnimFrequency(pstObject));
  }

  BIND(Object, set_anim_time)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_FLOAT(fTime, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetAnimTimeRecursive(pstObject, fTime);
    }
    else
    {
      orxObject_SetAnimTime(pstObject, fTime);
    }
    RETURN_NONE;
  }

  BIND(Object, get_anim_time)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetAnimTime(pstObject));
  }

  BIND(Object, set_current_anim)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetCurrentAnimRecursive(pstObject, zName);
    }
    else
    {
      orxObject_SetCurrentAnim(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, get_current_anim)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_STR(orxObject_GetCurrentAnim(pstObject));
  }

  BIND(Object, set_target_anim)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetTargetAnimRecursive(pstObject, zName);
    }
    else
    {
      orxObject_SetTargetAnim(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, get_target_anim)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_STR(orxObject_GetTargetAnim(pstObject));
  }

  BIND(Object, set_speed)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(pvSpeed, 1);
    ARG_BOOL(bRelative, 2);
    if (bRelative)
    {
      orxObject_SetRelativeSpeed(pstObject, pvSpeed);
    }
    else
    {
      orxObject_SetSpeed(pstObject, pvSpeed);
    }
    RETURN_NONE;
  }

  BIND(Object, get_speed)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bRelative, 1);
    orxVECTOR vSpeed = orxVECTOR_0;
    if (bRelative)
    {
      orxObject_GetRelativeSpeed(pstObject, &vSpeed);
    }
    else
    {
      orxObject_GetSpeed(pstObject, &vSpeed);
    }
    RETURN_VECTOR(&vSpeed);
  }

  BIND(Object, set_angular_velocity)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_FLOAT(fVelocity, 1);
    orxObject_SetAngularVelocity(pstObject, fVelocity);
    RETURN_NONE;
  }

  BIND(Object, get_angular_velocity)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetAngularVelocity(pstObject));
  }

  BIND(Object, set_custom_gravity)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_VECTOR(vGravity, 1);
    orxObject_SetCustomGravity(pstObject, vGravity);
    RETURN_NONE;
  }

  BIND(Object, get_custom_gravity)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxVECTOR vGravity = orxVECTOR_0;
    orxObject_GetCustomGravity(pstObject, &vGravity);
    RETURN_VECTOR(&vGravity);
  }

  BIND(Object, get_mass)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetMass(pstObject));
  }

  BIND(Object, get_mass_center)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxVECTOR vCenter = orxVECTOR_0;
    orxObject_GetMassCenter(pstObject, &vCenter);
    RETURN_VECTOR(&vCenter);
  }

  BIND(Object, apply_torque)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_FLOAT(fTorque, 1);
    orxObject_ApplyTorque(pstObject, fTorque);
    RETURN_NONE;
  }

  BIND(Object, apply_force)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(vForce, 1);
    ARG_VECTOR(vPoint, 2);
    orxObject_ApplyForce(pstObject, vForce, vPoint);
    RETURN_NONE;
  }

  BIND(Object, apply_impulse)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(vImpulse, 1);
    ARG_VECTOR(vPoint, 2);
    orxObject_ApplyImpulse(pstObject, vImpulse, vPoint);
    RETURN_NONE;
  }

  BIND(Object, raycast)
  {
    PY_CHECK_ARGC(5);
    ARG_VECTOR(vBegin, 0);
    ARG_VECTOR(vEnd, 1);
    ARG_INT(u16SelfFlags, 2);
    ARG_INT(u16CheckMask, 3);
    ARG_BOOL(bEarlyExit, 4);
    orxVECTOR vContact;
    orxVECTOR vNormal;
    orxOBJECT *pstDetected = orxObject_Raycast(vBegin, vEnd, u16SelfFlags, u16CheckMask, bEarlyExit, &vContact, &vNormal);
    if (pstDetected != orxNULL)
    {
      py_newtuple(py_retval(), 3);
      py_neworxobject(py_tuple_getitem(py_retval(), 0), pstDetected);
      py_neworxvector(py_tuple_getitem(py_retval(), 1), &vContact);
      py_neworxvector(py_tuple_getitem(py_retval(), 2), &vNormal);
      return true;
    }
    else
    {
      RETURN_NONE;
    }
  }

  BIND(Object, set_text_string)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zString, 1);
    orxObject_SetTextString(pstObject, zString);
    RETURN_NONE;
  }

  BIND(Object, get_text_string)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_STR(orxObject_GetTextString(pstObject));
  }

  BIND(Object, add_fx)
  {
    PY_CHECK_ARGC(5);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    ARG_BOOL(bUnique, 3);
    ARG_FLOAT(fPropagationDelay, 4);
    if (bRecursive && bUnique)
    {
      orxObject_AddUniqueFXRecursive(pstObject, zName, fPropagationDelay);
    }
    else if (bRecursive)
    {
      orxObject_AddFXRecursive(pstObject, zName, fPropagationDelay);
    }
    else if (bUnique)
    {
      orxObject_AddUniqueFX(pstObject, zName);
    }
    else
    {
      orxObject_AddFX(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, remove_fx)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_RemoveFXRecursive(pstObject, zName);
    }
    else
    {
      orxObject_RemoveFX(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, remove_all_fxs)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bRecursive, 1);
    if (bRecursive)
    {
      orxObject_RemoveAllFXsRecursive(pstObject);
    }
    else
    {
      orxObject_RemoveAllFXs(pstObject);
    }
    RETURN_NONE;
  }

  BIND(Object, add_sound)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zName, 1);
    orxObject_AddSound(pstObject, zName);
    RETURN_NONE;
  }

  BIND(Object, remove_sound)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zName, 1);
    orxObject_RemoveSound(pstObject, zName);
    RETURN_NONE;
  }

  BIND(Object, remove_all_sounds)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_RemoveAllSounds(pstObject);
    RETURN_NONE;
  }

  BIND(Object, set_volume)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_FLOAT(fVolume, 1);
    orxObject_SetVolume(pstObject, fVolume);
    RETURN_NONE;
  }

  BIND(Object, set_pitch)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_FLOAT(fPitch, 1);
    orxObject_SetPitch(pstObject, fPitch);
    RETURN_NONE;
  }

  BIND(Object, set_panning)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_FLOAT(fPanning, 1);
    ARG_BOOL(bMix, 2);
    orxObject_SetPanning(pstObject, fPanning, bMix);
    RETURN_NONE;
  }

  BIND(Object, play)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_Play(pstObject);
    RETURN_NONE;
  }

  BIND(Object, stop)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_Stop(pstObject);
    RETURN_NONE;
  }

  BIND(Object, add_filter)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_STR(zName, 1);
    orxObject_AddFilter(pstObject, zName);
    RETURN_NONE;
  }

  BIND(Object, remove_last_filter)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_RemoveLastFilter(pstObject);
    RETURN_NONE;
  }

  BIND(Object, remove_all_filters)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxObject_RemoveAllFilters(pstObject);
    RETURN_NONE;
  }

  BIND(Object, set_shader_from_config)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR_OR_NONE(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetShaderFromConfigRecursive(pstObject, zName);
    }
    else
    {
      orxObject_SetShaderFromConfig(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, enable_shader)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bEnabled, 1);
    orxObject_EnableShader(pstObject, bEnabled);
    RETURN_NONE;
  }

  BIND(Object, is_shader_enabled)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_BOOL(orxObject_IsShaderEnabled(pstObject));
  }

  BIND(Object, add_time_line_track)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_AddTimeLineTrackRecursive(pstObject, zName);
    }
    else
    {
      orxObject_AddTimeLineTrack(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, remove_time_line_track)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_RemoveTimeLineTrackRecursive(pstObject, zName);
    }
    else
    {
      orxObject_RemoveTimeLineTrack(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, enable_time_line)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bEnable, 1);
    orxObject_EnableTimeLine(pstObject, bEnable);
    RETURN_NONE;
  }

  BIND(Object, is_time_line_enabled)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_BOOL(orxObject_IsTimeLineEnabled(pstObject));
  }

  BIND(Object, add_trigger)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_AddTriggerRecursive(pstObject, zName);
    }
    else
    {
      orxObject_AddTrigger(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, remove_trigger)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_STR(zName, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_RemoveTriggerRecursive(pstObject, zName);
    }
    else
    {
      orxObject_RemoveTrigger(pstObject, zName);
    }
    RETURN_NONE;
  }

  BIND(Object, fire_trigger)
  {
    PY_CHECK_ARGC(4);
    OBJECT;
    ARG_STR(zName, 1);

    orxU32 u32RefinementListLength = 0;
    const orxSTRING azRefinementList[32];

    if (py_islist(py_arg(2)))
    {
      PY_CHECK_ARG_TYPE(2, tp_list);
      py_ObjectRef pyRefinementList = py_arg(2);
      u32RefinementListLength = py_list_len(pyRefinementList);
      for (orxS32 i = 0; i < u32RefinementListLength; ++i)
      {
        py_ObjectRef pyRefinement = py_list_getitem(pyRefinementList, i);
        if (!py_checkstr(pyRefinement))
        {
          return false;
        }
        azRefinementList[i] = py_tostr(pyRefinement);
      }
    }

    ARG_BOOL(bRecursive, 3);
    if (bRecursive)
    {
      orxObject_FireTriggerRecursive(pstObject, zName, u32RefinementListLength > 0 ? (const orxSTRING *)azRefinementList : orxNULL, u32RefinementListLength);
    }
    else
    {
      orxObject_FireTrigger(pstObject, zName, u32RefinementListLength > 0 ? (const orxSTRING *)azRefinementList : orxNULL, u32RefinementListLength);
    }
    RETURN_NONE;
  }

  BIND(Object, get_name)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_STR(orxObject_GetName(pstObject));
  }

  BIND(Object, set_rgb)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_VECTOR(pvRGB, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetRGBRecursive(pstObject, pvRGB);
    }
    else
    {
      orxObject_SetRGB(pstObject, pvRGB);
    }
    RETURN_NONE;
  }

  BIND(Object, get_rgb)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxVECTOR vRGB = orxVECTOR_0;
    orxObject_GetRGB(pstObject, &vRGB);
    RETURN_VECTOR(&vRGB);
  }

  BIND(Object, set_alpha)
  {
    PY_CHECK_ARGC(3);
    OBJECT;
    ARG_FLOAT(fAlpha, 1);
    ARG_BOOL(bRecursive, 2);
    if (bRecursive)
    {
      orxObject_SetAlphaRecursive(pstObject, fAlpha);
    }
    else
    {
      orxObject_SetAlpha(pstObject, fAlpha);
    }
    RETURN_NONE;
  }

  BIND(Object, get_alpha)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetAlpha(pstObject));
  }

  BIND(Object, set_life_time)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    py_Ref pyLifeTime = py_arg(1);
    if (py_isfloat(pyLifeTime) || py_isint(pyLifeTime))
    {
      ARG_FLOAT(fLifeTime, 1);
      orxObject_SetLifeTime(pstObject, fLifeTime);
    }
    else if (py_isnone(pyLifeTime))
    {
      orxObject_SetLifeTime(pstObject, -orxFLOAT_1);
    }
    else if (py_isstr(pyLifeTime))
    {
      ARG_STR(zLifeTime, 1);
      orxObject_SetLiteralLifeTime(pstObject, zLifeTime);
    }
    else
    {
      return ValueError("Object life time can be one of None, int, float, or str. Got %s.", py_tpname(py_typeof(pyLifeTime)));
    }
    RETURN_NONE;
  }

  BIND(Object, get_life_time)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    orxFLOAT fLifeTime = orxObject_GetLifeTime(pstObject);
    if (fLifeTime < orxFLOAT_0)
    {
      RETURN_NONE;
    }
    else
    {
      RETURN_FLOAT(fLifeTime);
    }
  }

  BIND(Object, get_active_time)
  {
    PY_CHECK_ARGC(1);
    OBJECT;
    RETURN_FLOAT(orxObject_GetActiveTime(pstObject));
  }

  BIND(Object, reset_active_time)
  {
    PY_CHECK_ARGC(2);
    OBJECT;
    ARG_BOOL(bRecursive, 1);
    if (bRecursive)
    {
      orxObject_ResetActiveTimeRecursive(pstObject);
    }
    else
    {
      orxObject_ResetActiveTime(pstObject);
    }
    RETURN_NONE;
  }

  // GUID functions

  BIND(Guid, __str__)
  {
    PY_CHECK_ARGC(1);
    ARG_GUID(u64GUID, 0);
    orxCHAR acInstanceName[20];
    orxString_NPrint(acInstanceName, sizeof(acInstanceName), "0x%016llX", u64GUID);
    RETURN_STR(acInstanceName);
  }

  // Config functions

  BIND(Config, set_parent)
  {
    PY_CHECK_ARGC(2);
    ARG_STR(zSection, 0);
    ARG_STR_OR_NONE(zParent, 1);
    orxConfig_SetParent(zSection, zParent);
    RETURN_NONE;
  }

  BIND(Config, get_parent)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zSection, 0);
    const orxSTRING zParent = orxConfig_GetParent(zSection);
    if (zParent == orxNULL)
    {
      RETURN_NONE;
    }
    else
    {
      RETURN_STR(zParent);
    }
  }

  BIND(Config, push_section)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zSection, 0);
    orxConfig_PushSection(zSection);
    RETURN_NONE;
  }

  BIND(Config, pop_section)
  {
    PY_CHECK_ARGC(0);
    orxConfig_PopSection();
    RETURN_NONE;
  }

  BIND(Config, context__new__)
  {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_newobject(py_retval(), py_totype(py_arg(0)), 1, 0);
    py_setslot(py_retval(), 0, py_arg(1));
    return true;
  }

  BIND(Config, context__enter__)
  {
    PY_CHECK_ARGC(1);
    orxConfig_PushSection(py_tostr(py_getslot(py_arg(0), 0)));
    RETURN_NONE;
  }

  BIND(Config, context__exit__)
  {
    PY_CHECK_ARGC(1);
    orxConfig_PopSection();
    RETURN_NONE;
  }

  BIND(Config, get_sections)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    py_Ref pyResult = py_retval();
    orxU32 u32Count = orxConfig_GetSectionCount();
    py_newlistn(pyResult, u32Count);
    for (orxU32 u32Index = 0; u32Index < u32Count; ++u32Index)
    {
      py_newstr(py_offset(pyResult, u32Index), orxConfig_GetSection(u32Index));
    }
    return true;
  }

  BIND(Config, get_keys)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    py_Ref pyResult = py_retval();
    orxU32 u32Count = orxConfig_GetKeyCount();
    py_newlistn(pyResult, u32Count);
    for (orxU32 u32Index = 0; u32Index < u32Count; ++u32Index)
    {
      py_newstr(py_offset(pyResult, u32Index), orxConfig_GetKey(u32Index));
    }
    return true;
  }

  BIND(Config, get_list_count)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_INT(orxConfig_GetListCount(zName));
  }

#define BIND_CONFIG_SET_GET(type, argtype, name)                         \
  BIND(Config, set_##type)                                               \
  {                                                                      \
    PY_CHECK_ARGC(2);                                                    \
    ARG_STR(zKey, 0);                                                    \
    ARG_##argtype(name##_value, 1);                                      \
    orxConfig_Set##name(zKey, name##_value);                             \
    RETURN_NONE;                                                         \
  }                                                                      \
  BIND(Config, get_##type)                                               \
  {                                                                      \
    PY_CHECK_ARGC(2);                                                    \
    ARG_STR(zKey, 0);                                                    \
    py_Ref pyIndex = py_arg(1);                                          \
    if (py_isnone(pyIndex))                                              \
    {                                                                    \
      RETURN_##argtype(orxConfig_Get##name(zKey));                       \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      ARG_INT(s32Index, 1);                                              \
      RETURN_##argtype(orxConfig_GetList##name(zKey, (orxS32)s32Index)); \
    }                                                                    \
  }

  BIND_CONFIG_SET_GET(bool, BOOL, Bool)
  BIND_CONFIG_SET_GET(int, INT, S64)
  BIND_CONFIG_SET_GET(guid, GUID, U64)
  BIND_CONFIG_SET_GET(float, FLOAT, Float)
  BIND_CONFIG_SET_GET(string, STR, String)

  BIND(Config, set_vector)
  {
    PY_CHECK_ARGC(2);
    ARG_STR(zKey, 0);
    ARG_VECTOR(pvValue, 1);
    orxConfig_SetVector(zKey, pvValue);
    RETURN_NONE;
  }

  BIND(Config, get_vector)
  {
    PY_CHECK_ARGC(2);
    ARG_STR(zKey, 0);
    py_Ref pyIndex = py_arg(1);

    orxVECTOR vValue = orxVECTOR_0;
    if (py_isnone(pyIndex))
    {
      orxConfig_GetVector(zKey, &vValue);
    }
    else
    {
      ARG_INT(s32Index, 1);
      orxConfig_GetListVector(zKey, (orxS32)s32Index, &vValue);
    }

    RETURN_VECTOR(&vValue);
  }

#undef BIND_CONFIG_SET_GET

  BIND(Config, has_section)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zSection, 0);
    RETURN_BOOL(orxConfig_HasSection(zSection));
  }

  BIND(Config, has_value)
  {
    PY_CHECK_ARGC(2);
    ARG_STR(zKey, 0);
    ARG_BOOL(bCheckSpelling, 1);
    if (bCheckSpelling)
    {
      RETURN_BOOL(orxConfig_HasValue(zKey));
    }
    else
    {
      RETURN_BOOL(orxConfig_HasValueNoCheck(zKey));
    }
    return py_exception(tp_AssertionError, "Impossible situation");
  }

  BIND(Config, clear_section)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zSection, 0);
    orxConfig_ClearSection(zSection);
    RETURN_NONE;
  }

  BIND(Config, clear_value)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zKey, 0);
    orxConfig_ClearValue(zKey);
    RETURN_NONE;
  }

  // Command functions

  BIND(Command, evaluate)
  {
    ARG_STR(zCommand, 0);
    orxCOMMAND_VAR stResult;
    if (py_isnone(py_arg(1)))
    {
      orxCommand_Evaluate(zCommand, &stResult);
    }
    else
    {
      ARG_GUID(u64GUID, 1);
      orxCommand_EvaluateWithGUID(zCommand, u64GUID, &stResult);
    }
    RETURN_NONE;
  }

  // Input functions

  BIND(Input, push_set)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    orxInput_PushSet(zName);
    RETURN_NONE;
  }

  BIND(Input, pop_set)
  {
    PY_CHECK_ARGC(0);
    orxInput_PopSet();
    RETURN_NONE;
  }

  BIND(Input, context__new__)
  {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_newobject(py_retval(), py_totype(py_arg(0)), 1, 0);
    py_setslot(py_retval(), 0, py_arg(1));
    return true;
  }

  BIND(Input, context__enter__)
  {
    PY_CHECK_ARGC(1);
    orxInput_PushSet(py_tostr(py_getslot(py_arg(0), 0)));
    RETURN_NONE;
  }

  BIND(Input, context__exit__)
  {
    PY_CHECK_ARGC(1);
    orxInput_PopSet();
    RETURN_NONE;
  }

  BIND(Input, enable_set)
  {
    PY_CHECK_ARGC(2);
    ARG_STR(zName, 0);
    ARG_BOOL(bEnable, 1);
    orxInput_EnableSet(zName, bEnable);
    RETURN_NONE;
  }

  BIND(Input, is_set_enabled)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_BOOL(orxInput_IsSetEnabled(zName));
  }

  BIND(Input, get_all)
  {
    PY_CHECK_ARGC(0);
    py_Ref pyResult = py_retval();
    py_newlist(pyResult);
    for (const orxSTRING zInput = orxInput_GetNext(orxNULL); zInput; zInput = orxInput_GetNext(zInput))
    {
      py_newstr(py_r0(), zInput);
      py_list_append(pyResult, py_r0());
    }
    return true;
  }

  BIND(Input, is_active)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_BOOL(orxInput_IsActive(zName));
  }

  BIND(Input, has_been_activated)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_BOOL(orxInput_HasBeenActivated(zName));
  }

  BIND(Input, has_been_deactivated)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_BOOL(orxInput_HasBeenDeactivated(zName));
  }

  BIND(Input, get_value)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    RETURN_FLOAT(orxInput_GetValue(zName));
  }

  BIND(Input, set_value)
  {
    PY_CHECK_ARGC(3);
    ARG_STR(zName, 0);
    ARG_FLOAT(fValue, 1);
    ARG_BOOL(bPermanent, 2);
    if (bPermanent)
    {
      orxInput_SetPermanentValue(zName, fValue);
    }
    else
    {
      orxInput_SetValue(zName, fValue);
    }
    RETURN_NONE;
  }

  BIND(Input, reset_value)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zName, 0);
    orxInput_ResetValue(zName);
    RETURN_NONE;
  }

  // Type wrappers

  BIND(Vector, __new__)
  {
    PY_CHECK_ARGC(4);
    ARG_FLOAT(fX, 1);
    ARG_FLOAT(fY, 2);
    ARG_FLOAT(fZ, 3);
    orxVECTOR *pvVector = (orxVECTOR *)py_newobject(py_retval(), tp_orxvector, 0, sizeof(orxVECTOR));
    pvVector->fX = fX;
    pvVector->fY = fY;
    pvVector->fZ = fZ;
    return true;
  }

  BIND(Vector, __eq__)
  {
    PY_CHECK_ARGC(2);
    ARG_VECTOR(pvSelf, 0);
    ARG_VECTOR(pvOther, 1);
    RETURN_BOOL(orxVector_AreEqual(pvSelf, pvOther));
  }

  BIND(Vector, __neg__)
  {
    PY_CHECK_ARGC(1);
    ARG_VECTOR(pvSelf, 0);
    orxVECTOR vResult;
    orxVector_Neg(&vResult, pvSelf);
    RETURN_VECTOR(&vResult);
  }

  BIND(Vector, copy)
  {
    PY_CHECK_ARGC(1);
    ARG_VECTOR(pvSelf, 0);
    orxVECTOR vCopy;
    orxVector_Copy(&vCopy, pvSelf);
    RETURN_VECTOR(&vCopy);
  }

#define BIND_VECTOR_OP(name, func) \
  BIND(Vector, name)               \
  {                                \
    PY_CHECK_ARGC(2);              \
    ARG_VECTOR(vSelf, 0);          \
    ARG_VECTOR(vOp, 1);            \
    orxVECTOR vRes = orxVECTOR_0;  \
    func(&vRes, vSelf, vOp);       \
    RETURN_VECTOR(&vRes);          \
  }

#define BIND_VECTOR_MOP(name, func)         \
  BIND(Vector, name)                        \
  {                                         \
    PY_CHECK_ARGC(2);                       \
    ARG_VECTOR(vSelf, 0);                   \
    py_Ref pyOp = py_arg(1);                \
    orxVECTOR vRes = orxVECTOR_0;           \
    if (py_isfloat(pyOp) || py_isint(pyOp)) \
    {                                       \
      ARG_FLOAT(fOp, 1);                    \
      func##f(&vRes, vSelf, fOp);           \
    }                                       \
    else                                    \
    {                                       \
      ARG_VECTOR(vOp, 1);                   \
      func(&vRes, vSelf, vOp);              \
    }                                       \
    RETURN_VECTOR(&vRes);                   \
  }

  BIND_VECTOR_OP(__add__, orxVector_Add)
  BIND_VECTOR_OP(__sub__, orxVector_Sub)
  BIND_VECTOR_MOP(__mul__, orxVector_Mul)
  BIND_VECTOR_MOP(__truediv__, orxVector_Div)

#undef BIND_VECTOR_OP
#undef BIND_VECTOR_MOP

#define BIND_VECTOR_PROPERTY(property) \
  BIND(Vector, get_##property)         \
  {                                    \
    PY_CHECK_ARGC(1);                  \
    ARG_VECTOR(pvSelf, 0);             \
    RETURN_FLOAT(pvSelf->f##property); \
  }                                    \
  BIND(Vector, set_##property)         \
  {                                    \
    PY_CHECK_ARGC(2);                  \
    ARG_VECTOR(pvSelf, 0);             \
    ARG_FLOAT(f##property, 1);         \
    pvSelf->f##property = f##property; \
    RETURN_NONE;                       \
  }

  BIND_VECTOR_PROPERTY(X)
  BIND_VECTOR_PROPERTY(Y)
  BIND_VECTOR_PROPERTY(Z)

#undef BIND_VECTOR_PROPERTY

  BIND(Vector, size)
  {
    PY_CHECK_ARGC(1);
    ARG_VECTOR(vOp, 0);
    RETURN_FLOAT(orxVector_GetSize(vOp));
  }

  BIND(Vector, distance)
  {
    PY_CHECK_ARGC(2);
    ARG_VECTOR(vOp1, 0);
    ARG_VECTOR(vOp2, 1);
    RETURN_FLOAT(orxVector_GetDistance(vOp1, vOp2));
  }

  BIND(Vector, lerp)
  {
    PY_CHECK_ARGC(3);
    ARG_VECTOR(vOp1, 0);
    ARG_VECTOR(vOp2, 1);
    ARG_FLOAT(fOp, 2);
    orxVECTOR vRes = orxVECTOR_0;
    orxVector_Lerp(&vRes, vOp1, vOp2, fOp);
    RETURN_VECTOR(&vRes);
  }

  BIND(Vector, rotate_in_place)
  {
    PY_CHECK_ARGC(2);
    ARG_VECTOR(vSelf, 0);
    ARG_FLOAT(fAngle, 1);
    orxVector_2DRotate(vSelf, vSelf, fAngle);
    RETURN_NONE;
  }

  BIND(Vector, rotate)
  {
    PY_CHECK_ARGC(2);
    ARG_VECTOR(vOp, 0);
    ARG_FLOAT(fAngle, 1);
    orxVECTOR vRes = orxVECTOR_0;
    orxVector_2DRotate(&vRes, vOp, fAngle);
    RETURN_VECTOR(&vRes);
  }

  BIND(Vector, normalize_in_place)
  {
    PY_CHECK_ARGC(1);
    ARG_VECTOR(vSelf, 0);
    orxVector_Normalize(vSelf, vSelf);
    RETURN_NONE;
  }

  BIND(Vector, normalize)
  {
    PY_CHECK_ARGC(1);
    ARG_VECTOR(vOp, 0);
    orxVECTOR vRes = orxVECTOR_0;
    orxVector_Normalize(&vRes, vOp);
    RETURN_VECTOR(&vRes);
  }

  BIND(Clock, compute_dt)
  {
    PY_CHECK_ARGC(2);
    ARG_CLOCK(pstClock, 0);
    ARG_FLOAT(fDT, 1);
    RETURN_FLOAT(orxClock_ComputeDT(pstClock, fDT));
  }

  void orxPy_AddModuleClock()
  {
    py_Ref pyMod = py_newmodule("orx.clock");
    py_Type pyType = py_newtype("Clock", tp_object, pyMod, orxNULL);

    tp_orxclock = pyType;

    py_bindmethod(pyType, "compute_dt", BINDING_NAME(Clock, compute_dt));
  }

  void orxPy_AddModuleVector()
  {
    py_Ref pyMod = py_newmodule("orx.vector");
    py_Type pyType = py_newtype("Vector", tp_object, pyMod, orxNULL);

    tp_orxvector = pyType;

    py_bindmagic(pyType, __new__, BINDING_NAME(Vector, __new__));
    py_bindmagic(pyType, __eq__, BINDING_NAME(Vector, __eq__));
    py_bindmagic(pyType, __neg__, BINDING_NAME(Vector, __neg__));
    py_bindmagic(pyType, __add__, BINDING_NAME(Vector, __add__));
    py_bindmagic(pyType, __sub__, BINDING_NAME(Vector, __sub__));
    py_bindmagic(pyType, __mul__, BINDING_NAME(Vector, __mul__));
    py_bindmagic(pyType, __truediv__, BINDING_NAME(Vector, __truediv__));

    py_bindproperty(pyType, "x", BINDING_NAME(Vector, get_X), BINDING_NAME(Vector, set_X));
    py_bindproperty(pyType, "y", BINDING_NAME(Vector, get_Y), BINDING_NAME(Vector, set_Y));
    py_bindproperty(pyType, "z", BINDING_NAME(Vector, get_Z), BINDING_NAME(Vector, set_Z));

    py_bindmethod(pyType, "copy", BINDING_NAME(Vector, copy));

    py_bindmethod(pyType, "size", BINDING_NAME(Vector, size));
    py_bindmethod(pyType, "rotate", BINDING_NAME(Vector, rotate_in_place));
    py_bindmethod(pyType, "normalize", BINDING_NAME(Vector, normalize_in_place));

    py_bindfunc(pyMod, "rotate", BINDING_NAME(Vector, rotate));
    py_bindfunc(pyMod, "normalize", BINDING_NAME(Vector, normalize));
    py_bindfunc(pyMod, "distance", BINDING_NAME(Vector, distance));
    py_bindfunc(pyMod, "lerp", BINDING_NAME(Vector, lerp));
  }

  void orxPy_AddModuleGuid()
  {
    py_Ref pyMod = py_newmodule("orx.guid");
    py_Type pyType = py_newtype("Guid", tp_object, pyMod, orxNULL);

    tp_orxguid = pyType;

    py_bindmagic(pyType, __str__, BINDING_NAME(Guid, __str__));
  }

  void orxPy_AddModuleObject()
  {
    py_Ref pyMod = py_newmodule("orx.object");
    py_Type pyType = py_newtype("Object", tp_object, pyMod, orxNULL);

    tp_orxobject = pyType;

    py_bindmagic(pyType, __new__, BINDING_NAME(Object, __new__));
    py_bindmagic(pyType, __eq__, BINDING_NAME(Object, __eq__));
    py_bindmagic(pyType, __hash__, BINDING_NAME(Object, __hash__));
    py_bindmagic(pyType, __str__, BINDING_NAME(Object, __str__));

#define BIND_METHOD(sig, name) py_bind(py_tpobject(pyType), sig, BINDING_NAME(Object, name))

    BIND_METHOD("delete(self) -> None", delete);
    BIND_METHOD("get_guid(self) -> Guid", get_guid);

    BIND_METHOD("enable(self, state: bool, recursive: bool = False) -> None", enable);
    BIND_METHOD("is_enabled(self) -> bool", is_enabled);

    BIND_METHOD("pause(self, state: bool, recursive: bool = False) -> None", pause);
    BIND_METHOD("is_paused(self) -> bool", is_paused);

    BIND_METHOD("set_owner(self, owner: Object | None) -> None", set_owner);
    BIND_METHOD("get_owner(self) -> Object | None", get_owner);

    BIND_METHOD("find_owned_child(self, path: str) -> Object | None", find_owned_child);

    BIND_METHOD("set_clock(self, clock: Clock, recursive: bool = False)", set_clock);
    BIND_METHOD("get_clock(self) -> Clock | None", get_clock);

    BIND_METHOD("set_flip(self, flip_x: bool, flip_y: bool) -> None", set_flip);
    BIND_METHOD("get_flip(self) -> tuple[bool, bool]", get_flip);

    BIND_METHOD("set_position(self, position: Vector, world: bool = False) -> None", set_position);
    BIND_METHOD("get_position(self, world: bool = False) -> Vector", get_position);

    BIND_METHOD("set_rotation(self, rotation: float, world: bool = False) -> None", set_rotation);
    BIND_METHOD("get_rotation(self, world: bool = False) -> float", get_rotation);

    BIND_METHOD("set_scale(self, scale: Vector, world: bool = False) -> None", set_scale);
    BIND_METHOD("get_scale(self, world: bool = False) -> Vector", get_scale);

    BIND_METHOD("set_parent(self, parent: Object | None) -> None", set_parent);
    BIND_METHOD("get_parent(self) -> Object | None", get_parent);

    BIND_METHOD("find_child(self, path: str) -> Object | None", find_child);

    BIND_METHOD("attach(self, parent: Object) -> None", attach);
    BIND_METHOD("detach(self) -> None", detach);

    BIND_METHOD("log_parents(self) -> None", log_parents);

    BIND_METHOD("set_anim_frequency(self, frequency: float, recursive: bool = False) -> None", set_anim_frequency);
    BIND_METHOD("get_anim_frequency(self) -> float", get_anim_frequency);

    BIND_METHOD("set_anim_time(self, time: float, recursive: bool = False) -> None", set_anim_time);
    BIND_METHOD("get_anim_time(self) -> float", get_anim_time);

    BIND_METHOD("set_current_anim(self, name: str, recursive: bool = False) -> None", set_current_anim);
    BIND_METHOD("get_current_anim(self) -> str", get_current_anim);

    BIND_METHOD("set_target_anim(self, name: str, recursive: bool = False) -> None", set_target_anim);
    BIND_METHOD("get_target_anim(self) -> str", get_target_anim);

    BIND_METHOD("set_speed(self, speed: Vector, relative = False) -> None", set_speed);
    BIND_METHOD("get_speed(self, relative: bool = False) -> Vector", get_speed);

    BIND_METHOD("set_angular_velocity(self, velocity: float) -> None", set_angular_velocity);
    BIND_METHOD("get_angular_velocity(self) -> float", get_angular_velocity);

    BIND_METHOD("set_custom_gravity(self, dir: Vector) -> None", set_custom_gravity);
    BIND_METHOD("get_custom_gravity(self) -> Vector", get_custom_gravity);

    BIND_METHOD("get_mass(self) -> float", get_mass);
    BIND_METHOD("get_mass_center(self) -> Vector", get_mass_center);

    BIND_METHOD("apply_torque(self, torque: float) -> None", apply_torque);
    BIND_METHOD("apply_force(self, force: Vector, point: Vector) -> None", apply_force);
    BIND_METHOD("apply_impulse(self, impulse: Vector, point: Vector) -> None", apply_impulse);

    BIND_METHOD("set_text_string(self, s: str) -> None", set_text_string);
    BIND_METHOD("get_text_string(self) -> str", get_text_string);

    BIND_METHOD("add_fx(self, name: str, recursive: bool = False, unique: bool = True, propagation_delay: float = 0) -> None", add_fx);
    BIND_METHOD("remove_fx(self, name: str, recursive: bool = False) -> None", remove_fx);
    BIND_METHOD("remove_all_fxs(self, recursive: bool = False) -> None", remove_all_fxs);

    BIND_METHOD("add_sound(self, name: str) -> None", add_sound);
    BIND_METHOD("remove_sound(self, name: str) -> None", remove_sound);
    BIND_METHOD("remove_all_sounds(self) -> None", remove_all_sounds);

    BIND_METHOD("set_volume(self, volume: float) -> None", set_volume);
    BIND_METHOD("set_pitch(self, pitch: float) -> None", set_pitch);
    BIND_METHOD("set_panning(self, panning: float, mix: bool) -> None", set_panning);

    BIND_METHOD("play(self) -> None", play);
    BIND_METHOD("stop(self) -> None", stop);

    BIND_METHOD("add_filter(self, name: str) -> None", add_filter);
    BIND_METHOD("remove_last_filter(self) -> None", remove_last_filter);
    BIND_METHOD("remove_all_filters(self) -> None", remove_all_filters);

    BIND_METHOD("set_shader_from_config(self, name: str | None, recursive: bool = False) -> None", set_shader_from_config);
    BIND_METHOD("enable_shader(self, enabled: bool = True) -> None", enable_shader);
    BIND_METHOD("is_shader_enabled(self) -> bool", is_shader_enabled);

    BIND_METHOD("add_time_line_track(self, name: str, recusive: bool = False) -> None", add_time_line_track);
    BIND_METHOD("remove_time_line_track(self, name: str, recursive: bool = False) -> None", remove_time_line_track);
    BIND_METHOD("enable_time_line(self, enabled: bool = True) -> None", enable_time_line);
    BIND_METHOD("is_time_line_enabled(self) -> bool", is_time_line_enabled);

    BIND_METHOD("add_trigger(self, name: str, recursive: bool = False) -> None", add_trigger);
    BIND_METHOD("remove_trigger(self, name: str, recursive: bool = False) -> None", remove_trigger);
    BIND_METHOD("fire_trigger(self, name: str, refinement: list[str] | None = None, recursive: bool = False) -> None", fire_trigger);

    BIND_METHOD("get_name(self) -> str", get_name);

    BIND_METHOD("set_rgb(self, rgb: Vector, recursive: bool = False) -> None", set_rgb);
    BIND_METHOD("get_rgb(self) -> Vector", get_rgb);

    BIND_METHOD("set_alpha(self, alpha: float, recursive: bool = False) -> None", set_alpha);
    BIND_METHOD("get_alpha(self) -> float", get_alpha);

    BIND_METHOD("set_life_time(self, life_time: float | str | None) -> None", set_life_time);
    BIND_METHOD("get_life_time(self) -> float | None", get_life_time);

    BIND_METHOD("get_active_time(self) -> float", get_active_time);
    BIND_METHOD("reset_active_time(self, recursive: bool = False) -> None", reset_active_time);

#undef BIND_METHOD

    // Bind object functions
    py_bind(pyMod, "create_from_config(section_name: str) -> Object | None", BINDING_NAME(Object, __new__));
    py_bind(pyMod, "from_guid(guid: Guid) -> Object | None", BINDING_NAME(Object, __new__));
    py_bind(pyMod, "raycast(begin: Vector, end: Vector, self_flags: int, check_mask: int, early_exit: bool = False) -> tuple[Object, Vector, Vector] | None", BINDING_NAME(Object, raycast));
  }

  BIND(Orx, log)
  {
    PY_CHECK_ARGC(1);
    ARG_STR(zMessage, 0);
    orxLOG("%s", zMessage);
    RETURN_NONE;
  }

  BIND(Orx, close)
  {
    PY_CHECK_ARGC(0);
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    RETURN_NONE;
  }

#undef BIND
#undef ARG
#undef ARG_OR_NONE
#undef ARG_STR
#undef ARG_INT
#undef ARG_BOOL
#undef ARG_PTR
#undef ARG_OBJECT
#undef ARG_VECTOR
#undef OBJECT
#undef RETURN
#undef RETURN_NONE
#undef RETURN_OR_NONE
#undef RETURN_STR
#undef RETURN_FLOAT
#undef RETURN_INT
#undef RETURN_BOOL

  orxSTATUS orxPy_ExecSource(const orxSTRING zPath)
  {
    orxSTATUS eResult = orxSTATUS_FAILURE;

    // Load source
    orxCHAR *pBuffer = orxPy_ReadSource(zPath);

    if (pBuffer != orxNULL)
    {
      py_StackRef pyStackPos = py_peek(0);

      if (py_exec(pBuffer, zPath, EXEC_MODE, orxNULL))
      {
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxPy_LogException();
        py_clearexc(pyStackPos);
      }

      orxMemory_Free((void *)pBuffer);
    }

    return eResult;
  }

  void orxPy_AddModuleOrx()
  {
    py_Ref pyMod = py_newmodule("orx");

    py_bind(pyMod, "log(message: str) -> None", BINDING_NAME(Orx, log));
    py_bind(pyMod, "close() -> None", BINDING_NAME(Orx, close));
  }

  void orxPy_AddModuleConfig()
  {
    // Register config module
    py_Ref pyMod = py_newmodule("orx.config");

#define BIND_FUNC(sig, name) py_bind(pyMod, sig, BINDING_NAME(Config, name))

    py_Type pyContextType = py_newtype("Section", tp_object, pyMod, orxNULL);

    py_bindmagic(pyContextType, __new__, BINDING_NAME(Config, context__new__));
    py_bindmagic(pyContextType, __enter__, BINDING_NAME(Config, context__enter__));
    py_bindmagic(pyContextType, __exit__, BINDING_NAME(Config, context__exit__));

    // Bind config functions
    BIND_FUNC("set_parent(name: str, parent: str | None) -> None", set_parent);
    BIND_FUNC("get_parent(name: str) -> str | None", get_parent);
    BIND_FUNC("push_section(name: str) -> None", push_section);
    BIND_FUNC("pop_section() -> None", pop_section);

    BIND_FUNC("get_sections() -> list[str]", get_sections);
    BIND_FUNC("get_keys() -> list[str]", get_keys);

    BIND_FUNC("get_list_count(key: str) -> int", get_list_count);

    BIND_FUNC("set_bool(key: str, value: bool) -> None", set_bool);
    BIND_FUNC("get_bool(key: str, index: int | None = None) -> bool", get_bool);
    BIND_FUNC("set_int(key: str, value: int) -> None", set_int);
    BIND_FUNC("get_int(key: str, index: int | None = None) -> int", get_int);
    BIND_FUNC("set_guid(key: str, value: Guid) -> None", set_guid);
    BIND_FUNC("get_guid(key: str, index: int | None = None) -> Guid", get_guid);
    BIND_FUNC("set_float(key: str, value: float) -> None", set_float);
    BIND_FUNC("get_float(key: str, index: int | None = None) -> float", get_float);
    BIND_FUNC("set_string(key: str, value: str) -> None", set_string);
    BIND_FUNC("get_string(key: str, index: int | None = None) -> str", get_string);
    BIND_FUNC("set_vector(key: str, value: Vector) -> None", set_vector);
    BIND_FUNC("get_vector(key: str, index: int | None = None) -> Vector", get_vector);

    BIND_FUNC("has_section(name: str) -> bool", has_section);
    BIND_FUNC("has_value(key: str, check_spelling: bool = True) -> bool", has_value);

    BIND_FUNC("clear_section(name: str) -> None", clear_section);
    BIND_FUNC("clear_value(key: str) -> None", clear_value);

#undef BIND_FUNC
  }

  void orxPy_AddModuleCommand()
  {
    // Register command module
    py_Ref pyMod = py_newmodule("orx.command");

#define BIND_FUNC(sig, name) py_bind(pyMod, sig, BINDING_NAME(Command, name))

    // Bind command functions
    BIND_FUNC("evaluate(command: str, guid: Guid | None = None) -> None", evaluate);

#undef BIND_FUNC
  }

  void orxPy_AddModuleInput()
  {
    // Register command module
    py_Ref pyMod = py_newmodule("orx.input");

#define BIND_FUNC(sig, name) py_bind(pyMod, sig, BINDING_NAME(Input, name))

    py_Type pyContextType = py_newtype("Set", tp_object, pyMod, orxNULL);

    py_bindmagic(pyContextType, __new__, BINDING_NAME(Input, context__new__));
    py_bindmagic(pyContextType, __enter__, BINDING_NAME(Input, context__enter__));
    py_bindmagic(pyContextType, __exit__, BINDING_NAME(Input, context__exit__));

    // Bind input functions
    BIND_FUNC("push_set(name: str) -> None", push_set);
    BIND_FUNC("pop_set() -> None", pop_set);
    BIND_FUNC("enable_set(name: str, enable: bool = True) -> None", enable_set);
    BIND_FUNC("is_set_enabled(name: str) -> bool", is_set_enabled);

    BIND_FUNC("get_all() -> list[str]", get_all);

    BIND_FUNC("is_active(name: str) -> bool", is_active);
    BIND_FUNC("has_been_activated(name: str) -> bool", has_been_activated);
    BIND_FUNC("has_been_deactivated(name: str) -> bool", has_been_deactivated);

    BIND_FUNC("get_value(name: str) -> float", get_value);
    BIND_FUNC("set_value(name: str, value: float, permanent: bool = False) -> None", set_value);
    BIND_FUNC("reset_value(name: str) -> None", reset_value);

#undef BIND_FUNC
  }

#undef BINDING_NAME

  void orxPy_AddModules()
  {
    orxPy_AddModuleOrx();
    orxPy_AddModuleClock();
    orxPy_AddModuleVector();
    orxPy_AddModuleGuid();
    orxPy_AddModuleConfig();
    orxPy_AddModuleCommand();
    orxPy_AddModuleInput();
    orxPy_AddModuleObject();
  }

  orxSTATUS orxPy_EventHandler(const orxEVENT *_pstEvent)
  {
    orxPROFILER_PUSH_MARKER(__FUNCTION__);

    orxSTATUS eResult = orxSTATUS_SUCCESS;

    switch (_pstEvent->eType)
    {
    case orxEVENT_TYPE_OBJECT:
    {
      switch (_pstEvent->eID)
      {
      case orxOBJECT_EVENT_CREATE:
      {
        if (sstPy.pyOnCreate != 0)
        {
          py_neworxobject(py_r0(), (orxOBJECT *)_pstEvent->hSender);
          // Call handler
          orxPy_Call1(orxPy_GetCallback(sstPy.pyOnCreate), py_r0());
        }
        break;
      }
      case orxOBJECT_EVENT_DELETE:
      {
        if (sstPy.pyOnDelete != 0)
        {
          py_neworxobject(py_r0(), (orxOBJECT *)_pstEvent->hSender);
          // Call handler
          orxPy_Call1(orxPy_GetCallback(sstPy.pyOnDelete), py_r0());
        }
        break;
      }
      default:
      {
        orxLOG(orxPY_KZ_LOG_TAG "orxPy_EventHandler: Unsupported object event %d", _pstEvent->eID);
        orxASSERT(orxFALSE);
        break;
      }
      }
      break;
    }
    case orxEVENT_TYPE_PHYSICS:
    {
      orxPROFILER_PUSH_MARKER("orxPy_EventHandler_Physics");
      switch (_pstEvent->eID)
      {
      case orxPHYSICS_EVENT_CONTACT_ADD:
      {
        if (sstPy.pyOnCollide != 0)
        {
          orxPHYSICS_EVENT_PAYLOAD *pstPayload = (orxPHYSICS_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          // Build argument list
          orxS32 s32Argc = 6;
          py_Ref pyArgv = py_r0();
          py_newlistn(pyArgv, s32Argc);
          py_neworxobject(py_offset(pyArgv, 0), (orxOBJECT *)_pstEvent->hSender);
          py_newstr(py_offset(pyArgv, 1), orxBody_GetPartName(pstPayload->pstSenderPart));
          py_neworxobject(py_offset(pyArgv, 2), (orxOBJECT *)_pstEvent->hRecipient);
          py_newstr(py_offset(pyArgv, 3), orxBody_GetPartName(pstPayload->pstRecipientPart));
          py_neworxvector(py_offset(pyArgv, 4), &pstPayload->vPosition);
          py_neworxvector(py_offset(pyArgv, 5), &pstPayload->vNormal);

          // Call handler
          orxPy_CallN(orxPy_GetCallback(sstPy.pyOnCollide), s32Argc, pyArgv);
        }
        break;
      }
      case orxPHYSICS_EVENT_CONTACT_REMOVE:
      {
        if (sstPy.pyOnSeparate != 0)
        {
          orxPHYSICS_EVENT_PAYLOAD *pstPayload = (orxPHYSICS_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          // Build argument list
          orxS32 s32Argc = 4;
          py_Ref pyArgv = py_r0();
          py_newlistn(pyArgv, s32Argc);
          py_neworxobject(py_offset(pyArgv, 0), (orxOBJECT *)_pstEvent->hSender);
          py_newstr(py_offset(pyArgv, 1), orxBody_GetPartName(pstPayload->pstSenderPart));
          py_neworxobject(py_offset(pyArgv, 2), (orxOBJECT *)_pstEvent->hRecipient);
          py_newstr(py_offset(pyArgv, 3), orxBody_GetPartName(pstPayload->pstRecipientPart));
          // Call handler
          orxPy_CallN(orxPy_GetCallback(sstPy.pyOnSeparate), s32Argc, pyArgv);
        }
        break;
      }
      default:
      {
        orxLOG(orxPY_KZ_LOG_TAG "orxPy_EventHandler: Unsupported physics event %d", _pstEvent->eID);
        orxASSERT(orxFALSE);
        break;
      }
      }
      orxPROFILER_POP_MARKER();
      break;
    }
    case orxEVENT_TYPE_SHADER:
    {
      orxPROFILER_PUSH_MARKER("orxPy_EventHandler_Shader");
      orxASSERT(_pstEvent->eID == orxSHADER_EVENT_SET_PARAM);
      orxSHADER_EVENT_PAYLOAD *pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;
      if (sstPy.pyOnShaderParam != 0 && (pstPayload->eParamType == orxSHADER_PARAM_TYPE_FLOAT || pstPayload->eParamType == orxSHADER_PARAM_TYPE_VECTOR))
      {
        // Build argument list
        orxS32 s32Argc = 4;
        py_Ref pyArgv = py_r0();
        py_newlistn(pyArgv, s32Argc);
        py_neworxobject(py_offset(pyArgv, 0), (orxOBJECT *)_pstEvent->hSender);
        py_newstr(py_offset(pyArgv, 1), pstPayload->zShaderName);
        py_newstr(py_offset(pyArgv, 2), pstPayload->zParamName);
        py_assign(py_offset(pyArgv, 3), pstPayload->eParamType == orxSHADER_PARAM_TYPE_FLOAT ? py_tpobject(tp_float) : py_tpobject(tp_orxvector));

        // Call handler
        orxSTATUS eShaderResult = orxPy_CallN(orxPy_GetCallback(sstPy.pyOnShaderParam), s32Argc, pyArgv);
        if (eShaderResult == orxSTATUS_SUCCESS && (py_isfloat(py_retval()) || py_isint(py_retval()) || py_istype(py_retval(), tp_orxvector)))
        {
          switch (pstPayload->eParamType)
          {
          case orxSHADER_PARAM_TYPE_FLOAT:
          {
            if (py_isfloat(py_retval()) || py_isint(py_retval()))
            {
              py_castfloat32(py_retval(), &pstPayload->fValue);
            }
            break;
          }
          case orxSHADER_PARAM_TYPE_VECTOR:
          {
            if (py_istype(py_retval(), tp_orxvector))
            {
              orxVECTOR *pvVal = (orxVECTOR *)py_touserdata(py_retval());
              orxVector_Copy(&pstPayload->vValue, pvVal);
            }
            break;
          }
          default:
          {
            // An unsupported parameter type
            break;
          }
          }
        }
      }
      orxPROFILER_POP_MARKER();
      break;
    }
    default:
    {
      orxLOG(orxPY_KZ_LOG_TAG "orxPy_EventHandler: Unsupported event (%d, %d)", _pstEvent->eType, _pstEvent->eID);
      orxASSERT(orxFALSE);
      break;
    }
    }

    orxPROFILER_POP_MARKER();

    // Done!
    return eResult;
  }

  orxSTATUS orxPy_InitVM()
  {
    orxSTATUS eResult = orxSTATUS_SUCCESS;

    // Not already initialized?
    if (sstPy.bInitialized == orxFALSE)
    {
      // Init variables
      orxMemory_Zero(&(sstPy), sizeof(orxPY));
      sstPy.pyMod = orxNULL;
      sstPy.pyInit = 0;
      sstPy.pyUpdate = 0;
      sstPy.pyExit = 0;
      sstPy.pyOnCreate = 0;
      sstPy.pyOnDelete = 0;
      sstPy.pyOnCollide = 0;
      sstPy.pyOnDelete = 0;
      sstPy.pyOnShaderParam = 0;

      // Initialize VM, with or without os support enabled
      py_initialize();

      // Setup hooks for imports
      py_Callbacks *pstCallbacks = py_callbacks();
      pstCallbacks->importfile = orxPy_ReadSource;
      pstCallbacks->print = orxPy_Print;

      // Add core orx modules to the VM
      orxPy_AddModules();

      // Update status
      sstPy.bInitialized = orxTRUE;
    }

    return eResult;
  }

#define SET_CALLBACK(target, name)                     \
  pyName = py_name(name);                              \
  pyCallback = py_getdict(pyMod, pyName);              \
  if (pyCallback != orxNULL && !py_isnone(pyCallback)) \
  {                                                    \
    target = pyName;                                   \
  }                                                    \
  else                                                 \
  {                                                    \
    target = 0;                                        \
  }

  void orxPy_InitCallbacks()
  {
    // Get orx module
    py_GlobalRef pyMod = py_getmodule("orx");
    py_Name pyName;
    py_ItemRef pyCallback = orxNULL;

    // Set module reference
    sstPy.pyMod = pyMod;

    // Get function names for functions which are defined
    SET_CALLBACK(sstPy.pyInit, orxPY_KZ_DEFAULT_INIT)
    SET_CALLBACK(sstPy.pyUpdate, orxPY_KZ_DEFAULT_UPDATE)
    SET_CALLBACK(sstPy.pyExit, orxPY_KZ_DEFAULT_EXIT)
    SET_CALLBACK(sstPy.pyOnCreate, orxPY_KZ_DEFAULT_OBJECT_CREATE_EVENT_HANDLER)
    SET_CALLBACK(sstPy.pyOnDelete, orxPY_KZ_DEFAULT_OBJECT_DELETE_EVENT_HANDLER)
    SET_CALLBACK(sstPy.pyOnCollide, orxPY_KZ_DEFAULT_PHYSICS_COLLIDE_EVENT_HANDLER)
    SET_CALLBACK(sstPy.pyOnSeparate, orxPY_KZ_DEFAULT_PHYSICS_SEPARATE_EVENT_HANDLER)
    SET_CALLBACK(sstPy.pyOnShaderParam, orxPY_KZ_DEFAULT_SHADER_PARAM_EVENT_HANDLER)

    // Register commands
    orxCOMMAND_REGISTER(orxPY_KZ_COMMAND_EXEC, orxPy_CommandPyExec, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 1, {"Source", orxCOMMAND_VAR_TYPE_STRING}, {"HideException? = false", orxCOMMAND_VAR_TYPE_BOOL});

    // Register event handler for events with handlers
    orxU32 u32ObjectAddFlags = orxEVENT_KU32_FLAG_ID_NONE;
    u32ObjectAddFlags |= sstPy.pyOnCreate != 0 ? orxEVENT_GET_FLAG(orxOBJECT_EVENT_CREATE) : orxEVENT_KU32_FLAG_ID_NONE;
    u32ObjectAddFlags |= sstPy.pyOnDelete != 0 ? orxEVENT_GET_FLAG(orxOBJECT_EVENT_DELETE) : orxEVENT_KU32_FLAG_ID_NONE;
    if (u32ObjectAddFlags != orxEVENT_KU32_FLAG_ID_NONE)
    {
      orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxPy_EventHandler);
      orxEvent_SetHandlerIDFlags(orxPy_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, u32ObjectAddFlags, orxEVENT_KU32_MASK_ID_ALL);
    }
    orxU32 u32PhysicsAddFlags = orxEVENT_KU32_FLAG_ID_NONE;
    u32PhysicsAddFlags |= sstPy.pyOnCollide != 0 ? orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_ADD) : orxEVENT_KU32_FLAG_ID_NONE;
    u32PhysicsAddFlags |= sstPy.pyOnSeparate != 0 ? orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_REMOVE) : orxEVENT_KU32_FLAG_ID_NONE;
    if (u32PhysicsAddFlags != orxEVENT_KU32_FLAG_ID_NONE)
    {
      orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, orxPy_EventHandler);
      orxEvent_SetHandlerIDFlags(orxPy_EventHandler, orxEVENT_TYPE_PHYSICS, orxNULL, u32PhysicsAddFlags, orxEVENT_KU32_MASK_ID_ALL);
    }
    orxU32 u32ShaderAddFlags = orxEVENT_KU32_FLAG_ID_NONE;
    u32ShaderAddFlags |= sstPy.pyOnShaderParam != 0 ? orxEVENT_GET_FLAG(orxSHADER_EVENT_SET_PARAM) : orxEVENT_KU32_FLAG_ID_NONE;
    if (u32ShaderAddFlags != orxEVENT_KU32_FLAG_ID_NONE)
    {
      orxEvent_AddHandler(orxEVENT_TYPE_SHADER, orxPy_EventHandler);
      orxEvent_SetHandlerIDFlags(orxPy_EventHandler, orxEVENT_TYPE_SHADER, orxNULL, u32ShaderAddFlags, orxEVENT_KU32_MASK_ID_ALL);
    }
  }

#undef SET_CALLBACK

  orxSTATUS orxPy_Init()
  {
    orxSTATUS eResult = orxSTATUS_SUCCESS;
    orxASSERT(sstPy.bInitialized == orxTRUE);

    orxConfig_PushSection(orxPY_KZ_CONFIG_SECTION);

    if (orxConfig_HasValue(orxPY_KZ_CONFIG_MAIN))
    {
      // Get Python main source location
      const orxSTRING zSource = orxConfig_GetString(orxPY_KZ_CONFIG_MAIN);

      // Read and compile main Python source
      eResult = orxPy_ExecSource(zSource);
    }

    orxConfig_PopSection();

    orxPy_InitCallbacks();

    if (sstPy.pyInit != 0)
    {
      // Call Python init function
      orxPy_Call(orxPy_GetCallback(sstPy.pyInit));
    }

    // Done!
    return eResult;
  }

  void orxPy_Exit()
  {
    if (sstPy.bInitialized == orxTRUE)
    {
      if (sstPy.pyExit != 0)
      {
        // Call Python exit function if it's registered
        orxPy_Call(orxPy_GetCallback(sstPy.pyExit));
      }

      // Free Python VM memory
      py_finalize();

      // Reset callback references
      sstPy.pyMod = orxNULL;
      sstPy.pyInit = 0;
      sstPy.pyUpdate = 0;
      sstPy.pyExit = 0;
      sstPy.pyOnCreate = 0;
      sstPy.pyOnDelete = 0;
      sstPy.pyOnCollide = 0;
      sstPy.pyOnSeparate = 0;
      sstPy.pyOnDelete = 0;
      sstPy.pyOnShaderParam = 0;

      // Remove event handler
      orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxPy_EventHandler);
      orxEvent_RemoveHandler(orxEVENT_TYPE_PHYSICS, orxPy_EventHandler);
      orxEvent_RemoveHandler(orxEVENT_TYPE_SHADER, orxPy_EventHandler);

      // Update status
      sstPy.bInitialized = orxFALSE;

      // Unregister Python commands
      orxCOMMAND_UNREGISTER(orxPY_KZ_COMMAND_EXEC);
    }
  }

#endif // orxPY_IMPL

#endif // _orxPY_H_

#ifdef __cplusplus
}
#endif /* __cplusplus */

echo Moving include files from Jet

md JetSDK
md JetSDK\include
md JetSDK\lib
attrib -R JetSDK\Include\*.*

copy ..\JetEngine\Jet.h                JetSDK\Include
copy ..\JetEngine\jeTypes.h            JetSDK\Include
copy ..\JetEngine\uvmap.h              JetSDK\Include
copy ..\JetEngine\camera.h             JetSDK\Include
copy ..\JetEngine\object.h             JetSDK\Include
copy ..\JetEngine\sound.h              JetSDK\Include
copy ..\JetEngine\sound3d.h            JetSDK\Include
copy ..\JetEngine\Support\basetype.h   JetSDK\Include
copy ..\JetEngine\Support\errorlog.h   JetSDK\Include
copy ..\JetEngine\Support\ram.h        JetSDK\Include
copy ..\JetEngine\Support\Array.h      JetSDK\Include
copy ..\JetEngine\Support\jeChain.h    JetSDK\Include
copy ..\JetEngine\Support\jePtrMgr.h   JetSDK\Include
copy ..\JetEngine\Support\jeNameMgr.h  JetSDK\Include
copy ..\JetEngine\Support\jeProperty.h JetSDK\Include
copy ..\JetEngine\Support\jeResource.h JetSDK\Include

copy ..\JetEngine\Math\quatern.h       JetSDK\Include
copy ..\JetEngine\Math\vec3d.h         JetSDK\Include
copy ..\JetEngine\Math\extbox.h        JetSDK\Include
copy ..\JetEngine\Math\xform3d.h       JetSDK\Include
copy ..\JetEngine\Actor\actor.h        JetSDK\Include
copy ..\JetEngine\Actor\body.h         JetSDK\Include
copy ..\JetEngine\Actor\motion.h       JetSDK\Include
copy ..\JetEngine\Actor\path.h         JetSDK\Include
copy ..\JetEngine\Actor\strblock.h     JetSDK\Include
copy ..\JetEngine\VFile\vfile.h        JetSDK\Include
copy ..\JetEngine\Bitmap\bitmap.h      JetSDK\Include
copy ..\JetEngine\Bitmap\pixelformat.h JetSDK\Include

copy ..\JetEngine\guWorld\juWorld.h     JetSDK\Include
copy ..\JetEngine\guWorld\juModel.h     JetSDK\Include
copy ..\JetEngine\guWorld\jeBrush.h     JetSDK\Include
copy ..\JetEngine\guWorld\jeFaceInfo.h  JetSDK\Include
copy ..\JetEngine\guWorld\jeVertArray.h JetSDK\Include
copy ..\JetEngine\guWorld\jeMaterial.h  JetSDK\Include
copy ..\JetEngine\guWorld\jeLight.h     JetSDK\Include
copy ..\JetEngine\guWorld\jePlane.h     JetSDK\Include
copy ..\JetEngine\guWorld\VisObject.h   JetSDK\Include
copy ..\JetEngine\guWorld\jeFrustum.h   JetSDK\Include
copy ..\JetEngine\guWorld\jeGArray.h    JetSDK\Include
copy ..\JetEngine\guWorld\jeUserPoly.h  JetSDK\Include
copy ..\JetEngine\guWorld\jePortal.h    JetSDK\Include
copy ..\JetEngine\guWorld\jePoly.h      JetSDK\Include
copy ..\JetEngine\guWorld\jeObjectIO.h  JetSDK\Include

copy ..\JetEngine\bsp\jebsp.h           JetSDK\Include

copy ..\JetEngine\Engine\engine.h       JetSDK\Include

REM if "%1" == "DEBUG" goto Debug
REM if "%1" == "RELEASE" goto Release
REM goto end

:Release
echo Moving Release dll and lib
attrib -R release\Jet3D.dll
attrib -R JetSDK\lib\Jet3D.lib
copy ..\JetEngine\ReleaseDLL\Jet3D.dll  release\astudio
copy ..\JetEngine\ReleaseDLL\Jet3D.lib  JetSDK\lib
REM goto end

:Debug
echo Moving Debug dll and lib
attrib -R Debug\Jet3Dd.dll
attrib -R JetSDK\lib\Jet3Dd.lib
copy ..\JetEngine\DebugDLL\Jet3Dd.dll   Debug\astudio
copy ..\JetEngine\DebugDLL\Jet3Dd.lib   JetSDK\lib

:end

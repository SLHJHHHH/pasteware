# pasteware

## Build

Requires Windows 10/11, Visual Studio with C++ workload (MSVC v143), Windows 10 SDK.

```
msbuild pasteware.vcxproj /p:Configuration=Release /p:Platform=Win32
```

Output: `Release/pasteware.dll`

Or open `pasteware.vcxproj` in Visual Studio, set Release + x86, build.

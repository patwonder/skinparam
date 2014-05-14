# SkinParam - Real-time skin renderer with adjustable skin parameters

## Introduction

This is a real-time skin renderer with adjustable skin parameters. The adjustable skin parameters
are listed as follows:

Parameter | Range | Description
--------- | ----- | -----------
mel | 0-50% | volume fraction of melanin in the epidermis
eum | 0-100% | percentage of eumelanin out of total melanin (the other is pheomelanin)
bld | 0-10% | volume fraction of hemoglobin in the dermis
ohg | 0-100% | percentage of oxyhemoglobin out of total hemoglobin (the other is deoxyhemoglobin)
Rough | 0.2-0.4 | roughness value, i.e. root-mean-square slope of the microfacets on skin surface

The renderer is based on
[SoG (Sum of Gaussians)](http://www.eugenedeon.com/?project=efficient-rendering-of-human-skin)
fitting of reflectance profiles in conjunction with a
[screen-space diffusion](http://www.iryoku.com/sssss/) technique. The SoG coefficients are
pre-computed offline and then interpolated in real-time according to skin parameters.

## Demo

You can find the pre-built binaries here:

https://github.com/patwonder/skinparam/tree/gh-pages/bin/

And a video demonstrating real-time adjusting of skin parameters:

https://github.com/patwonder/pbrt-v2-skin/raw/gh-pages/video/demo.mp4

## How to build

 1. Install the
[June 2010 DirectX SDK](http://www.microsoft.com/en-us/download/details.aspx?id=6812).
 2. Checkout the [pbrt-v2-skin](https://github.com/patwonder/pbrt-v2-skin) repository first. We need
the multipole computation utilities from pbrt-v2-skin. Make sure to place these 2 repositories
side by side in a same directory. We'll refer to them as
   * skinparam
   * pbrt-v2-skin
 3. Open pbrt-v2-skin/src/multipole/MultipoleProfileCalculator.sln, choose your configuration
(Release|Win32 recommended), and build.
 4. Open skinparam/proto1/SkinParam/SkinParam/DirectXTex/DirectXTex/DirectXTex_Desktop_2012.sln,
choose your configuration, and build.
 5. Open skinparam/proto1/SkinParam/SkinParam.sln, choose your configuration, and build.
 6. You need to copy the following folders into where the executable is built:
   * skinparam/proto1/SkinParam/SkinParam/Media
   * skinparam/proto1/SkinParam/SkinParam/model
   * skinparam/proto1/SkinParam/SkinParam/Shaders
 7. Run SkinParam.exe.

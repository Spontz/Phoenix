## Why

`Texture::load(std::vector<std::string>)` loads every slice of a 3D texture with `stbi_load`, which forces 8-bit output regardless of the source file, and then selects an internal format from the channel count only (`GL_R8`, `GL_RG8`, `GL_RGB8`, `GL_RGBA8`). Scientific volume data is commonly 16-bit and occupies a narrow part of that range: the `cthead` dataset shipped in `data/pool` is 16-bit grayscale whose maximum sample value is 3139 of 65535, so after the 8-bit conversion the whole volume collapses into roughly 12 distinct levels. Volume ray-marching shaders that classify samples against a density threshold cannot produce a usable surface from 12 levels, and the artist has no way to know which threshold values are meaningful because the engine never reports the data range.

The same load path also builds a full mipmap chain for every 3D texture. A ray-marcher samples a volume at positions unrelated to screen-space derivatives, so the level the hardware selects is arbitrary and blurs the volume, while the chain costs a third of the texture memory and the time to generate it.

## What Changes

- Detect the bit depth of the image files that compose a 3D texture and load 16-bit sources without truncating them to 8 bits.
- Select a 16-bit normalized internal format (`GL_R16`, `GL_RG16`, `GL_RGB16`, `GL_RGBA16`) and upload with `GL_UNSIGNED_SHORT` when the source is 16-bit, keeping the existing 8-bit path unchanged for 8-bit sources.
- Reject a 3D texture whose slices do not share the same bit depth, matching the existing check for mismatched width, height, and channel count.
- Stop allocating and generating mipmaps for 3D textures, so a volume is always sampled at the level it was uploaded.
- Report the loaded volume's actual value range and the selected internal format in the load log, so a density threshold can be chosen from real numbers instead of by trial and error.

## Capabilities

### New Capabilities

- `volume-texture-fidelity`: Faithful upload of 3D textures loaded from image files, covering source bit depth, sampling levels, and load diagnostics.

### Modified Capabilities

- None.

## Impact

- Affected code: `Engine/src/core/renderer/Texture.h` and `Engine/src/core/renderer/Texture.cpp` (3D load path, `getImageProperties`, `upload3DtoGPU`).
- Video memory for a 16-bit volume grows: the `cthead` set of 113 slices at 256x256 goes from 9.4 MB with the current truncated, mipmapped upload to 14.1 MB. Dropping the mipmap chain absorbs part of the increase, leaving it at roughly 1.5x.
- 3D textures lose their mipmap chain. No shader in the repository samples a `sampler3D` with an explicit LOD, and the implicit level a ray-marcher selects is not meaningful, so this removes blur rather than detail.
- No new dependencies: `stbi_info`, `stbi_is_16_bit`, and `stbi_load_16` are already part of the vendored `stb_image`.
- No change to 2D texture loading, to `Texture::create3D`, or to the runtime 3D texture pool, which already allocates a single level.
- No section configuration changes: precision follows the source files, so existing `.spo` files load unchanged. A shader that assumed the truncated 8-bit range will see a different sample distribution and may need its density threshold retuned.

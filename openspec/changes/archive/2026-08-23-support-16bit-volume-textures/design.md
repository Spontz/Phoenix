## Context

See [proposal.md](proposal.md) for motivation. The relevant constraints in the current code:

- `Texture::load(std::vector<std::string>)` in `Engine/src/core/renderer/Texture.cpp` calls `getImageProperties` on the first file, allocates one `unsigned char` block of `width * height * components * depth`, and fills it slice by slice with `stbi_load`. `stbi_load` always returns 8-bit samples, so a 16-bit source is truncated during decoding, before any OpenGL call.
- `getImageProperties` decodes the whole first image with `stbi_load` only to read its width, height, and channel count, then frees it.
- `upload3DtoGPU(const unsigned char* data)` derives the internal format from `m_components` alone and always passes `GL_UNSIGNED_BYTE` to `glTextureSubImage3D`. `m_internalFormat` is stored because `bindImage` needs it.
- `upload3DtoGPU` computes `m_mipmapLevels` from width and height, passes it to `glTextureStorage3D`, calls `glGenerateTextureMipmap`, selects a `*_MIPMAP_*` minification filter, and scales `m_mem` by 1.33.
- `Texture::create3D` already allocates a single level, so the two 3D paths currently disagree on level count.
- `Texture::Properties` (`m_flip`, `m_useLinearFilter`, `m_type`) is the only per-texture configuration channel.
- The vendored `stb_image` already provides `stbi_info`, `stbi_is_16_bit`, and `stbi_load_16`.

## Goals / Non-Goals

**Goals:**

- Make the GPU representation of an image-based 3D texture follow from the source files alone, with no configuration to get wrong.
- Leave the 8-bit path equivalent to today apart from the mipmap removal, including channel handling and filtering.
- Keep `bindImage` and the runtime 3D pool (`create3D`) untouched.
- Make the diagnostic output sufficient to pick a density threshold without external tooling.

**Non-Goals:**

- Normalizing or window/levelling sample values at load time. The volume is uploaded as authored; remapping stays in the shader.
- Exposing the measured value range as a uniform or as a queryable API. This change only logs it.
- Extending precision handling to 2D textures, cubemaps, or `loadFromMem`. 2D textures keep their mipmap chain.
- Float internal formats (`GL_R16F`, `GL_R32F`) for file-based textures. Normalized integer formats cover the source data and keep hardware filtering behavior identical.
- Changing the 3D wrap mode, which stays `GL_REPEAT`.

## Decisions

### Detect bit depth per file, with no override

Under the loader, every slice is probed with `stbi_is_16_bit` and all slices must agree, mirroring the existing width/height/component consistency check. The agreed bit depth then selects `stbi_load_16` or `stbi_load` for every slice.

No configuration knob is exposed. The file already carries the answer, a declaration in a `.spo` can go stale and silently degrade the image, and the goal of this change is a faithful representation of the source. The only argument for an override is capping video memory, and the mipmap removal below already returns part of that cost; if a demo later needs the trade-off, a property can be added without changing this behavior, since the default would remain detection.

### Replace the decode in `getImageProperties` with `stbi_info`

`getImageProperties` gains the bit depth, and its implementation moves from `stbi_load` to `stbi_info` plus `stbi_is_16_bit`. Neither decodes pixel data, so probing becomes cheap enough to run on every slice rather than on the first one only, which is what the per-slice consistency check requires. This also removes one full image decode from every 3D texture load.

### Carry precision to the upload as bytes-per-component

`upload3DtoGPU` takes the raw bytes plus the resolved bit depth, and selects the internal format from the `(components, bit depth)` pair:

| components | 8-bit | 16-bit |
| --- | --- | --- |
| 1 | `GL_R8` | `GL_R16` |
| 2 | `GL_RG8` | `GL_RG16` |
| 3 | `GL_RGB8` | `GL_RGB16` |
| 4 | `GL_RGBA8` | `GL_RGBA16` |

The pixel type passed to `glTextureSubImage3D` becomes `GL_UNSIGNED_SHORT` for 16-bit. Sized normalized integer formats are used rather than float formats so shaders keep reading values in [0, 1] with unchanged hardware filtering, and so `m_internalFormat` stays valid for `bindImage`. `m_mem` multiplies by bytes per component, matching the accounting already used by `create3D`.

The staging buffer stays a single contiguous allocation, sized in bytes rather than in samples. `GL_UNPACK_ALIGNMENT` is already set to 1 around the upload, which is also correct for 2-byte samples.

### Allocate a single level for 3D textures

`glTextureStorage3D` is called with one level, `glGenerateTextureMipmap` is dropped from the 3D path, the minification filter becomes `GL_LINEAR` or `GL_NEAREST` to match magnification, and the 1.33 factor disappears from `m_mem`.

A mip level of a volume is only meaningful when the sampler can derive a footprint from the sample coordinates. A ray-marcher computes coordinates from a ray parameter, so neighbouring fragments in a quad can land far apart in the volume and the implicit LOD is effectively noise; the result is a volume that blurs unpredictably. Nothing in the repository samples a `sampler3D` with an explicit LOD, and `create3D` already allocates one level, so this also makes the two 3D paths consistent.

A `Properties` flag to keep the chain was considered and rejected: no current or plausible consumer benefits from it, and it would preserve a filter setting that produces wrong results by default.

### Measure the value range during the slice copy

The loader tracks the minimum and maximum sample while the slices are already in memory, and logs them normalized by the format maximum (255 or 65535) alongside the source bit depth and the selected internal format. This is one pass over data that is being copied anyway. It is logged rather than exposed through an API because the consumer is a person tuning a threshold, and an API would need to define behavior for multi-channel volumes and for textures loaded from memory.

## Risks / Trade-offs

- [A 16-bit volume doubles the per-level video memory] -> Removing the mipmap chain returns a third of the previous allocation, so the net growth for the `cthead` set is about 1.5x rather than 2x, and the load log already reports per-texture and total texture memory.
- [Removing mipmaps changes sampling for any existing 3D texture, not only volumes] -> The affected sampling was already unpredictable for ray-marched volumes, and no repository shader requests an explicit LOD from a `sampler3D`; the change is verified by loading the existing volume sections.
- [Shaders tuned against the truncated 8-bit data will see a different sample distribution after the fix] -> The change is visible in the load log, and the reported range lets thresholds be retuned from real values.
- [`GL_RGB16` is a poorly aligned format on some drivers and may be padded internally] -> Volume data in this engine is single-channel or four-channel; the three-channel entry exists for completeness and its memory report may differ from the driver's real allocation.
- [Per-slice probing adds one file open per slice] -> `stbi_info` reads only the header, and the change removes a full image decode and the mipmap generation from the same code path, so total load time does not regress.

## Migration Plan

1. Land the probing and precision changes with the 8-bit path unchanged.
2. Land the single-level 3D allocation in the same upload function.
3. Load the `cthead` volume and confirm from the log that it is detected as 16-bit, that the reported range matches the source data, and that the reported memory matches a single 16-bit level.
4. Roll back by reverting the loader and upload changes; asset files and `.spo` files are unaffected because no configuration surface changes.

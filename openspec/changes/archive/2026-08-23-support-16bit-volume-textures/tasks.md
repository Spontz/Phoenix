## 1. Image Probing

- [x] 1.1 Rewrite `Texture::getImageProperties` to use `stbi_info` plus `stbi_is_16_bit` instead of decoding the image, and return the source bit depth alongside width, height, and channel count.
- [x] 1.2 Store the resolved bit depth on `Texture` so the upload and the log can read it.

## 2. 3D Texture Loading

- [x] 2.1 In `Texture::load(std::vector<std::string>)`, probe every slice and fail with a per-file diagnostic when width, height, channel count, or bit depth disagree.
- [x] 2.2 Size the staging buffer in bytes rather than in samples, using the resolved bit depth.
- [x] 2.3 Decode each slice with `stbi_load_16` or `stbi_load` according to the resolved bit depth, copying into the staging buffer.
- [x] 2.4 Track the minimum and maximum sample value during the slice copy.

## 3. GPU Upload

- [x] 3.1 Extend `upload3DtoGPU` to take the resolved bit depth and select the internal format from the `(components, bit depth)` table in design.md.
- [x] 3.2 Pass `GL_UNSIGNED_SHORT` to `glTextureSubImage3D` for 16-bit uploads and keep `GL_UNSIGNED_BYTE` for 8-bit ones.
- [x] 3.3 Allocate one level in `glTextureStorage3D`, remove the `glGenerateTextureMipmap` call, and set the minification filter to `GL_LINEAR` or `GL_NEAREST` to match magnification.
- [x] 3.4 Compute `m_mem` from a single level and from bytes per component, removing the 1.33 mipmap factor from the 3D path.

## 4. Diagnostics

- [x] 4.1 Log the source bit depth, the selected internal format, and the normalized minimum and maximum sample values when a 3D texture finishes loading.

## 5. Validation

- [x] 5.1 Build the Engine target.
- [x] 5.2 Load the `cthead` volume and confirm from the log that it is detected as 16-bit and that the reported range matches the source data.
- [x] 5.3 Confirm the reported memory for that volume matches a single 16-bit level and no mipmap chain.
- [x] 5.4 Load an 8-bit image sequence as a 3D texture and confirm its internal format is unchanged and its reported memory dropped by the removed mipmap chain.
- [x] 5.5 Render an existing `drawVolumeImage` section and confirm the volume is no longer blurred by an arbitrary mip level.
- [x] 5.6 Confirm 2D texture loading still generates mipmaps and reports unchanged memory.
- [x] 5.7 Run `openspec validate support-16bit-volume-textures --strict` and record any validation limitation.

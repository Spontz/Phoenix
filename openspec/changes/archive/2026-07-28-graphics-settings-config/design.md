## Context

Phoenix already reads graphics settings from `data/config/graphics.spo` through `SpoReader`. The current mapped settings include `gl_fullscreen`, `gl_width`, `gl_height`, `gl_aspect`, `gl_vsync`, and generic FBO entries. Generic FBOs are configured in `Window::fboConfig` and recreated through `Window::InitFbos()`.

Cacablu needs to edit the same data through a modal graphics settings dialog and send the resulting configuration to Phoenix over the existing editor API surface.

## Goals

- Let Cacablu replace Phoenix graphics settings through a native editor API call.
- Keep Phoenix's runtime state and `data/config/graphics.spo` consistent after a successful call.
- Use the existing Phoenix FBO concepts without exposing effect-specific FBOs used by the demo engine.
- Provide deterministic validation errors that identify the invalid field or FBO row.

## Non-Goals

- Replacing Phoenix's complete window creation lifecycle.
- Editing effect-specific FBOs such as bloom or accumulation buffers.
- Adding direct browser filesystem access to Phoenix's `data` folder.

## API Contract

Phoenix SHALL expose:

- `GET /api/graphics`: returns the current normalized graphics configuration.
- `PUT /api/graphics`: validates, applies, and persists a complete graphics configuration replacement.

The request body is:

```json
{
  "requestId": "graphics-optional-client-id",
  "context": {
    "colorDepth": 32,
    "width": 640,
    "height": 400,
    "fullscreen": false,
    "vsync": true,
    "targetFps": 60
  },
  "fbos": [
    {
      "index": 0,
      "ratio": 1,
      "format": "RGB",
      "width": null,
      "height": null,
      "attachments": 2,
      "filter": "bilinear"
    }
  ]
}
```

`filter` maps to `fbo_N_useFilter` and accepts only `bilinear` or `none`. Phoenix responses include `ok`, `requestId`, and either the normalized config or an error `code`, `message`, and optional `details` array.

## Validation

- The payload MUST contain exactly 25 FBO entries with unique indexes from `0` through `24`.
- FBO indexes `0` through `19` MUST use a positive `ratio` and MUST NOT require explicit `width` or `height`.
- FBO indexes `20` through `24` MUST use positive explicit `width` and `height`.
- `format` MUST match a Phoenix generic FBO format: `RGB`, `RGBA`, `RGB_16F`, `RGBA_16F`, `RGB_32F`, `RGBA_32F`, `RG_16F`, `DEPTH`, `DEPTH_16F`, or `DEPTH_32F`.
- `attachments` MUST be a positive integer accepted by Phoenix for the selected format.
- `width`, `height`, and `ratio` MUST be finite positive integers where required.
- `colorDepth` MUST be one of the supported UI values; unsupported values are rejected rather than coerced.

## Persistence

Phoenix writes `data/config/graphics.spo` under its active data folder. The file MUST use the variable names consumed by `SpoReader`:

- `gl_fullscreen`
- `gl_width`
- `gl_height`
- `gl_aspect`
- `gl_vsync`
- `gl_colorDepth` when supported by the runtime model
- `fbo_N_ratio` for rows `0` through `19`
- `fbo_N_width` and `fbo_N_height` for rows `20` through `24`
- `fbo_N_format`
- `fbo_N_colorAttachments`
- `fbo_N_useFilter`

The write SHOULD be atomic by writing a temporary file in `data/config` and replacing `graphics.spo` after serialization succeeds.

## Runtime Apply

After validation, Phoenix updates its in-memory window properties and `Window::fboConfig`. It applies V-sync immediately through `Window::SetVSync`, applies size changes through the existing window size path, recalculates aspect ratio, and recreates generic FBOs without touching effect-specific FBOs.

If a context setting cannot be safely applied immediately on the current platform, Phoenix still persists the value and returns a `restartRequired` warning in the success response. Validation, disk write, and FBO recreation failures return `ok: false` and leave the previous runtime configuration active.

## Error Codes

- `invalid-graphics-config`: request schema or field validation failed.
- `graphics-write-failed`: Phoenix could not create or replace `data/config/graphics.spo`.
- `graphics-apply-failed`: Phoenix could not apply the accepted runtime configuration.
- `graphics-unavailable`: Phoenix has not initialized enough rendering state to apply graphics settings.

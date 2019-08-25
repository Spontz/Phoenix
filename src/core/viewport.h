#pragma once

#include <main.h>

struct Viewport {
	Viewport() = default;
	Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
		:
		x(x),
		y(y),
		width(width),
		height(height)
	{
	}

	// Enforce aspect, center viewport, keep maximum area (top/down or left/right "black bars")
	// given a render target size and a desired viewport aspect ratio.
	static Viewport FromRenderTarget(GLsizei rt_width, GLsizei rt_height, GLsizei vp_aspect_x, GLsizei vp_aspect_y){
		GLsizei width = rt_width;
	GLsizei height = rt_height;
	GLint x = 0;
	GLint y = 0;

	const float rt_aspect_ratio_float = static_cast<float>(rt_width) / static_cast<float>(rt_height);
	const float vp_aspect_ratio_float = static_cast<float>(vp_aspect_x) / static_cast<float>(vp_aspect_y);

	if (rt_aspect_ratio_float < vp_aspect_ratio_float) {
		height = static_cast<GLsizei>(static_cast<float>(height) * rt_aspect_ratio_float / vp_aspect_ratio_float);
		y = (rt_height - height) / 2;
	}
	else if (rt_aspect_ratio_float > vp_aspect_ratio_float) {
		width = static_cast<GLsizei>(static_cast<float>(width) / rt_aspect_ratio_float * vp_aspect_ratio_float);
		x = (rt_width - width) / 2;
	}

	return { x,y,width,height };
}

	float GetAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

	GLint	x;
	GLint	y;
	GLsizei	width;
	GLsizei	height;
};

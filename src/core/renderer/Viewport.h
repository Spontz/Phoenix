#pragma once

#include "main.h"

struct UIntFraction {
	unsigned int numerator;
	unsigned int denominator;

	GLfloat GetRatio() const {
		return static_cast<float>(numerator) / static_cast<float>(denominator);
	}
};

struct Viewport {
	// Enforce aspect, center viewport, keep maximum area (top/down or left/right "black bars")
	// given a render target size and a desired viewport aspect ratio.
	static Viewport FromRenderTargetAndAspectRatio(unsigned int rt_width, unsigned int rt_height, float viewport_aspect_ratio) {
		unsigned int w = rt_width;
		unsigned int h = rt_height;
		int x = 0;
		int y = 0;

		const float rt_aspect_ratio = static_cast<float>(rt_width) / static_cast<float>(rt_height);

		if (rt_aspect_ratio < viewport_aspect_ratio) {
			h = static_cast<unsigned int>(static_cast<float>(h) * rt_aspect_ratio / viewport_aspect_ratio);
			y = (rt_height - h) / 2;
		}
		else if (rt_aspect_ratio > viewport_aspect_ratio) {
			w = static_cast<unsigned int>(static_cast<float>(w) / rt_aspect_ratio * viewport_aspect_ratio);
			x = (rt_width - w) / 2;
		}

		return { x,y,w,h };
	}

	static Viewport FromRenderTargetAndAspectFraction(unsigned int rt_width, unsigned int rt_height, UIntFraction const& viewport_aspect) {
		return FromRenderTargetAndAspectRatio(rt_width, rt_height, viewport_aspect.GetRatio());
	}

	float GetAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

	int				x;
	int				y;
	unsigned int	width;
	unsigned int	height;
};

#pragma once

#include "main.h"

#include <vector>

namespace sections {
	class SPH : public Section {

	private:
		typedef float real;
		typedef glm::vec<3, real> v3;

		struct Particle {
			v3 p_;
			v3 v_;
			v3 a_;
			v3 f_;
		};

	private:
		bool load();
		void init();
		void exec();
		void Draw();
		void Simulate();
		void end();

	private:
		static constexpr real	kSimFreqHz = 60; // hz
		static constexpr size_t	kBounds = 100;

	private:
		std::vector<Particle>	Particles_;
	};

}

#pragma once

#include "main.h"

#include <map>
#include <vector>

// Smoothed Particle Hydrodynamics section
// A coupled particle system simulator + renderer
// Lagrangian method of Navier-Stokes eq. integration
// Trivial acceleration via 3D grid
// Reference C++ simulator (CS later).

// 1) Simulates fluid interaction

// 2) Renders spheres as (z-aware) projection plane parallel rectangular billboards
//    (vs,gs,ps)->1 channel float32 rt (view space z)

// 3) Performs screen space operations to smooth and light ~isosurface
//    WIP: pos + normal reconstruction from z?

namespace Phoenix {
	class SPH : public Section {
		// IS units.

	private:
		typedef float real;
		typedef glm::vec<3, real> v3;

		struct Particle {
			v3 v;
			v3 a;
			v3 f;
		};

	private:
		bool load();
		void init();
		void exec();
		void Draw();
		void Simulate();

	private:
		static constexpr real	kSimFreqHz = 60; // hz
		typedef std::tuple<int, int, int> t_grid_index;
		typedef std::vector<size_t> t_grid_item;
		typedef std::map<t_grid_index, t_grid_item> t_grid;
		t_grid grid_;

	private:
		std::vector<v3>			particles_pos_; // separated for perfomance
		std::vector<Particle>	particles_;
		SP_Shader pass_sph_billboard_ = nullptr;
	};

}

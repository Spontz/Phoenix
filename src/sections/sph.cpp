#include "main.h"

#include "core/section.h"

namespace sections {
	class SPH : public Section {

	private:
		typedef float real;
		typedef glm::vec<3, real> v3;

		struct Particle {
			Particle(v3 const& p)
				:
				p_(p)
			{

			}

			v3 p_;
			v3 v_;
			v3 a_;
			v3 f_;
		};

	private:
		bool load() {

		}

		void init() {
			// hack: allow to provide initial state
			Particles_.clear();
			for (int x = -100; x <= 100; ++x)
				for (int y = -100; y <= 100; ++y)
					for (int z = -100; z <= 100; ++z)
						Particles_.push_back({ v3(x, y, z) });
		}

		void exec() {
			Simulate();
			Draw();
		}

		void Draw() {

		}

		void Simulate() {
			real elapsed = real(1) / real(SimulationHz_); // hack: get from engine

			std::vector<Particle*> grid[kBounds][kBounds][kBounds];
			memset(grid, 0, sizeof(grid));

			std::set<std::tuple<size_t, size_t, size_t>> visited;
			for (auto& i : Particles_) {
				if (i.p_.x <= real(0) || i.p_.x >= real(kBounds - 1) ||
					i.p_.y <= real(0) || i.p_.y >= real(kBounds - 1) ||
					i.p_.z <= real(0) || i.p_.z >= real(kBounds - 1)
					)
					continue;

				auto const x = static_cast<size_t>(i.p_.x);
				auto const y = static_cast<size_t>(i.p_.y);
				auto const z = static_cast<size_t>(i.p_.z);

				grid[x][y][z].push_back(&i);
				visited.insert({ x,y,z });
			}

			auto couple = [](Particle& dst, Particle const& src) {
				auto const dir = src.p_ - dst.p_;
				auto const f = real(1) / (dir.length() * dir.length()); // hack: use smoothing kernel
				dst.f_ += glm::normalize(dir) * f;
			};

			// hack: asuming particle mass == 1
			static constexpr real kMass = real(1);
			static constexpr real kFriction = real(.999);

			for (const auto& i : visited) {
				auto const x = std::get<0>(i);
				auto const y = std::get<1>(i);
				auto const z = std::get<2>(i);

				for (auto& j : grid[x][y][z]) {
					j->f_ = v3(0, 0, 0);

					for (auto& k : grid[x][y][z]) {
						if (j == k)
							continue;
						couple(*j, *k);
					}

					for (auto i_x = -1; i_x < 1; i_x += 2)
						for (auto i_y = -1; i_y < 1; i_y += 2)
							for (auto i_z = -1; i_z < 1; i_z += 2)
								for (auto& k : grid[i_x][i_y][i_z])
									couple(*j, *k);

					// hack: asuming particle mass == 1
					j->a_ = j->f_ / kMass;
					j->v_ += j->a_ / SimulationHz_;
					j->v_ *= kFriction;
					j->p_ += j->v_ / SimulationHz_;
				}
			}
		}

		void end() {

		}

	private:
		std::vector<Particle> Particles_;
		real SimulationHz_ = 60;
		static constexpr size_t kBounds = 100;
	};

}

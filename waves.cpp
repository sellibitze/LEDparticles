#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "common.hpp"

float squared(float x) { return x*x; }

/// Repelling force between two particles given their signed distance.
float kraft(float d) {
    if (d < 0) return -kraft(-d);
    if (d <= 2) {
        return 0.25f * d;
    }
    return squared(1.0f / d);
}

void update_accels(std::vector<particle>& ps) {
    if (ps.empty()) return;
    for (unsigned i = 0; i < ps.size(); ++i) { particle& p = ps[i];
        // Decellerate a little bit...
        p.acc = -0.0001f * p.vel;
    }
    const unsigned end = ps.size();
    // computing forces and accelerations across pairs of particles
    // that are not too far apart (up to the 5th neighrbour)...
    for (unsigned i = 0; i + 1 < end; ++i) {
        unsigned e2 = std::min(end, i + 5u);
        for (unsigned j = i + 1; j < e2; ++j) {
            particle& pi = ps[i];
            particle& pj = ps[j];
            float a = 0.03f * kraft(pj.pos - pi.pos);
            pi.acc -= a;
            pj.acc += a;
        }
    }
    // border particles don't move...
    ps.front().acc = 0.0;
    ps.back().acc = 0.0;
}

/// blend two colors
color fade(color from, color to, float pos) {
    color res;
    for (int k=0; k<3; ++k) {
        res.arr[k] = round_f2i(from.arr[k] * (1-pos) + to.arr[k] * pos);
    }
    return res;
}

int main() {
    // The physical computations run at about 1500 Hz. 30 consecutive
    // time steps are mixed into a frame buffer for motion blur. But
    // this is not only about motion blur. This higher temporal resolution
    // is necessary to avoid discretization errors to grow too large and
    // to keep the whole thing "stable".
    //
    // The framebuffer is "oversampled" (higher resolution) for
    // anti-aliasing.
    const int num_particles = 10;
    const int substeps = 30; // temporal oversampling
    const int sover = 5; // spatial oversampling

    std::srand(std::time(0));
    std::vector<particle> ps;
    ps.push_back( // bottom border particle that is not moving
        make_particle(
            make_random_color(),
            -1
        )
    );
    for (int i = 0; i < num_particles; ++i) {
        ps.push_back( // almost equidistantly spaced particles
            make_particle(
                make_random_color(),
                (i + 0.9f + 0.2f * uniform_random()) / (1 + num_particles) * (num_leds + 1) - 1
            )
        );
    }
    ps.push_back( // top border particle that is not moving
        make_particle(
            make_random_color(),
            num_leds
        )
    );

    std::vector<int> framebuff (num_leds * 3 * sover);
    std::vector<unsigned char> scratch;
    int time = -1;
    int period = 10000; // time steps until next impulse
    int part_change_color_index = 1;
    color part_change_color_new = {{0}}, part_change_color_old = {{0}};
#ifdef WRITE_PPM
    const int iters = 500; // for debugging purposes
    write_binary_ppm_header(num_leds, iters, std::cout);
    for (int i = 0; i < iters; ++i) {
#else
    for (;;) {
#endif
        // blank frame buffer (black)
        std::fill(framebuff.begin(), framebuff.end(), 0);
        for (int s = 0; s < substeps; ++s) {
            render_frame(ps, sover, substeps, framebuff);
            time = (time + 1) % period;
            if (time <= 1000) { // impulse phase
                if (time == 0) {
                    period = 5000 + std::rand() % 10000;
                    part_change_color_index = (std::rand() % num_particles) + 1;
                    part_change_color_old = ps[part_change_color_index].col;
                    part_change_color_new = make_random_color();
                }
                float s1 = squared(std::sin(time * (3.14159265 / 2000)));
                float s2 = squared(std::sin(time * (3.14159265 / 1000)));
                ps[0].pos = s2 * 15 - 1; // move bottom particle around
                // also change the color of a random particle once in a while
                ps[part_change_color_index].col = fade(
                    part_change_color_old,
                    part_change_color_new, s1);
            }
            update_accels(ps);
            progress(ps);
        }
        write_frame(framebuff, 1.0f/substeps, sover, scratch, std::cout);
#ifndef WRITE_PPM
        usleep(20000); // wait 20 ms (=> about 50 frames/sec)
#endif
    }
    return 0;
}

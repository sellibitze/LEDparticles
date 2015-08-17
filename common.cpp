#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "common.hpp"

namespace {

    void sort2(int& a, int& b) {
        if (b < a) {
            std::swap(a,b);
        }
    }

    template<class T>
    T clip(T min, T val, T max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

} // anonymous namespace

color make_color(unsigned char r, unsigned char g, unsigned char b) {
    color c = {{ r, g, b }};
    return c;
}

color make_random_color() {
    color c = {{0}};
    int z = std::rand() % 9;
    int first = z % 3; z /= 3;
    int secnd = z % 3; z /= 3;
    c.arr[secnd] = 85;
    c.arr[first] = 255;
    return c;
}

particle make_particle(color col, float pos, float vel) {
    particle p = { pos, vel, 0.0f, 0.0f, col };
    return p;
}

void progress(std::vector<particle>& ps) {
    // delta t = 1 (if you're wondering about units)
    for (unsigned i = 0; i < ps.size(); ++i) { particle& p = ps[i];
        // (extrapolated) average acceleration for current time step
        float a = p.acc * 1.5f - 0.5f * p.acc_old;
        // average velocity for current time step
        float v = p.vel + a * 0.5f;
        p.vel += a;
        p.pos += v;
        p.acc_old = p.acc;
    }
}

void render_frame(std::vector<particle> const& ps, float posmul, float v2pm,
                  std::vector<int>& framebuff) {
    int num_leds = framebuff.size() / 3;
    if (num_leds <= 0) return;
    unsigned end = ps.size();
    // from particles to pixels...
    for (unsigned pi = 0; pi < end; ++pi) {
        particle const& p = ps[pi];
        int led_index1 = round_f2i((p.pos - 0.5f * p.vel * v2pm) * posmul);
        int led_index2 = round_f2i((p.pos + 0.5f * p.vel * v2pm) * posmul);
        sort2(led_index1, led_index2);
        led_index1 -= 5;
        led_index2 += 5;
        float scale = 11.0f / (1 + led_index2 - led_index1);
        led_index1 = std::max(led_index1, 0);
        led_index2 = std::min(led_index2, num_leds - 1);
        for (int li = led_index1; li <= led_index2; ++li) {
            int* ptr = &framebuff[li * 3];
            for (int k = 0; k < 3; ++k) {
                ptr[k] += round_f2i(p.col.arr[k] * scale);
            }
        }
    }
}

float gamma(float x) {
#ifdef WRITE_PPM
    return std::sqrt(x/255.f)*255.f;
#else
    return x;
#endif
}

void write_frame(std::vector<int> const& framebuff, float scale, int decimate,
                 std::vector<unsigned char>& scratch_buff,
                 std::ostream& out) {
    int num_leds = framebuff.size() / 3 / decimate;
    scratch_buff.resize(num_leds * 3);
    scale /= decimate;
    for (int i = 0; i < num_leds; ++i) {
        int acc[3] = { 0 };
        for (int sub = 0; sub < decimate; ++sub) {
            int ofs = (i * decimate + sub) * 3;
            for (int k = 0; k < 3; ++k) {
                acc[k] += framebuff[ofs + k];
            }
        }
        int r = round_f2i(gamma(clip(0.f, acc[0] * scale, 255.f)));
        int g = round_f2i(gamma(clip(0.f, acc[1] * scale, 255.f)));
        int b = round_f2i(gamma(clip(0.f, acc[2] * scale, 255.f)));
        scratch_buff[i*3  ] = b; // LED strip wants blue component first,
        scratch_buff[i*3+1] = r; // then red,
        scratch_buff[i*3+2] = g; // and then green
    }
    out.write(reinterpret_cast<char const*>(&scratch_buff[0]), scratch_buff.size());
    out.flush();
}


/// writes an ASCII header for a binary PPM image.
void write_binary_ppm_header(unsigned width, unsigned height, std::ostream& out) {
    out << "P6\n" << width << ' ' << height << "\n255\n";
}

float uniform_random() {
    return static_cast<float>(std::rand()) / RAND_MAX;
}


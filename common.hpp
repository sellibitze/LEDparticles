#ifndef INC_GUARD_FSILQG1HCNAN65CHSMH5BH3T8Y1DR2
#define INC_GUARD_FSILQG1HCNAN65CHSMH5BH3T8Y1DR2

#include <ostream>
#include <vector>

const int num_leds =
#ifndef NUM_LEDS
    150;
#else
    NUM_LEDS;
#endif

struct color {
    unsigned char arr[3]; // R, G, B
};

struct particle {
    float pos; ///< position
    float vel; ///< velocity (d pos / d timestep)
    float acc; ///< acceleration (d vel / d timestep)
    float acc_old; ///< acceleration of previous time step
    color col;
};

/// round float to int
inline int round_f2i(float x) {
    return static_cast<int>(x + (x > 0 ? 0.5f : -0.5f));
}

/// create a custom color value
color make_color(unsigned char r, unsigned char g, unsigned char b);

/// create a random bright but fully saturated color
color make_random_color();

/// create a custom particle
particle make_particle(color col, float pos, float vel = 0.0f);

/// uniform random number from [0,1)
float uniform_random();

/// mutates particle vector in-place for the next time step
void progress(std::vector<particle>& ps);

/// writes an ASCII header for a binary PPM image.
void write_binary_ppm_header(unsigned width, unsigned height, std::ostream& out);

/// adds particles into framebuffer
void render_frame(std::vector<particle> const& ps,
                  float posmul, // spatial oversampling (for anti aliasing)
                  float v2pm,   // particle motion blur = velocity * v2pm
                  std::vector<int>& framebuff);

/// downsamples framebuffer and outputs it to the given output stream
void write_frame(std::vector<int> const& framebuff, float scale, int decimate,
                 std::vector<unsigned char>& scratch_buff,
                 std::ostream& out);

#endif /* include guard */

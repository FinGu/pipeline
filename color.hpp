
struct color{
    float r;
    float g;
    float b;
    float a;

    color(): r(0), g(0), b(0), a(0){}

    color(float r, float g, float b, float a): r(r), g(g), b(b), a(a){}

    color(float* f): r(f[0]), g(f[1]), b(f[2]), a(f[3]){}

    color operator +(const color &other){
        return color(r + other.r, g + other.g, b + other.b, a + other.a);
    }

    color &operator +=(const color &other){
        this->r += other.r;
        this->g += other.g;
        this->b += other.b;
        this->a += other.a;
        return *this;
    }

    color operator*(const color &other){
        return color(r * other.r, g * other.g, b * other.b, a * other.a);
    }

    color operator*(float other){
        return color(r * other, g * other, b * other, a * other);
    }

};


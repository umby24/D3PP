#pragma once
namespace D3PP::Common {
    struct Vector3F {
        float X;
        float Y;
        float Z;
    };

    struct Vector3S {
        short X;
        short Y;
        short Z;

        Vector3S() = default;
        Vector3S(short x, short y, short z) {
            X = x;
            Y = y;
            Z = z;
        }
        Vector3S(unsigned short x, unsigned short y, unsigned short z) {
            X = static_cast<short>(x);
            Y = static_cast<short>(y);
            Z = static_cast<short>(z);
        }

        bool isEqual(const Vector3S& other) const {
            return (X == other.X && Y == other.Y && Z == other.Z);
        }
    };
}


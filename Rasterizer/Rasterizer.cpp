// Rasterizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "TGA_Image.h"
#include "WavefrontReader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const Vec3f light_dir(0, 0, -1);
const int width = 500;
const int height = 500;


void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color) 
{
    int x0 = v0.x; int y0 = v0.y;
    int x1 = v1.x; int y1 = v1.y;

    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}



bool PointInPolygon(Vec2i point, std::vector<Vec2i> points)
{
    bool inside = false;

    for (int i = 0, j = points.size() - 1; i < points.size(); j = i++) {
        if (((points[i].y >= point.y) != (points[j].y >= point.y)) &&
            (point.x <= (points[j].x - points[i].x) * (point.y - points[i].y) / (points[j].y - points[i].y) + points[i].x))
        {
            inside = !inside;
        }
    }

    return inside;
}




//Vec3f barycentric(Vec2i* triangle, Vec2i point) {
//    Vec3f u = cross(Vec3f(triangle[2].x - triangle[0].x,  triangle[1].x -
//                          triangle[0].x,  triangle[0].x - point.x),
//                    Vec3f(triangle[2].y - triangle[0].y,  triangle[1].y -
//                          triangle[0].y,  triangle[0].y - point.y));
//
//    /* `triangle` and `point` has integer value as coordinates
//       so `abs(u[2])` < 1 means `u[2]` is 0, that means
//       triangle is degenerate, in this case return something with negative coordinates */
//    if (std::abs(u.z) < 1)
//    {
//        return Vec3f(-1, 1, 1);
//    }
//
//    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
//}


class TriangleRender
{
public:
    virtual void render_triangle( Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color) = 0;
};

class TriangleRenderFilled : public TriangleRender
{
public:
    //Here have another solution, probably more efficient.
//https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
    void render_triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color) {

        int minx = std::min(v0.x, std::min(v1.x, v2.x));
        int miny = std::min(v0.y, std::min(v1.y, v2.y));
        int maxx = std::max(v0.x, std::max(v1.x, v2.x));
        int maxy = std::max(v0.y, std::max(v1.y, v2.y));

        std::vector<Vec2i> triangle = std::vector<Vec2i>();
        triangle.push_back(v0);
        triangle.push_back(v1);
        triangle.push_back(v2);

        for (int x = minx; x <= maxx; x++)
        {
            for (int y = miny; y <= maxy; y++)
            {
                if (PointInPolygon(Vec2i(x, y), triangle))
                {
                    image.set(x, y, color);
                }
            }
        }
    }
};


class TriangleRenderWireframe : public TriangleRender
{
public:

    void render_triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color) {
        line(v0, v1, image, color);
        line(v1, v2, image, color);
        line(v2, v0, image, color);
    }
};

Vec3f triangle_normal(Vec3f v0, Vec3f v1, Vec3f v2)
{
    Vec3f normal = (v2 - v0) ^ (v1 - v0);

    return normal.normalize();
}




void model_render(WavefrontReader& model, TGAImage& image, TGAColor color, TriangleRender* renderer)
{
    const float scale = .9;
    const Vec3f object_to_world = Vec3f(1, 1, 0);

    Vec2i screen_coords[3];
    Vec3f world_coords[3];

    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        
        for (int j = 0; j < 3; j++) {
            Vec3f object_coordinate = model.vert(face[j]);
            Vec3f world_coordinate = (model.vert(face[j]) + object_to_world) * scale;

            world_coords[j] = world_coordinate;
            screen_coords[j] = Vec2i(world_coordinate.x * width / 2., world_coordinate.y * height / 2.);
        }

        float intensity = triangle_normal(world_coords[0], world_coords[1], world_coords[2]) * light_dir;
        intensity *= 255;

        TGAColor face_color = TGAColor(intensity, intensity,intensity, 255);

        if (intensity > 0) 
        {
            renderer->render_triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, face_color);
        }
    }
}



int main()
{
    TGAImage image(width, height, TGAImage::RGB);

    WavefrontReader* model = new WavefrontReader("Face.obj");

    TriangleRender* filled_renderer = new TriangleRenderFilled;
    TriangleRender* wire_renderer = new TriangleRenderWireframe;

    model_render(*model, image, red, filled_renderer);
    //model_render(*model, image, white, wire_renderer);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("Output.tga");

    free(wire_renderer);
    free(filled_renderer);
    free(model);
    return 0;
}

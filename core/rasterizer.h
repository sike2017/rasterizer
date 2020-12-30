#pragma once
#include "camera.h"
#include "primitive.h"
#include "zbuffer.h"
#include "display/WindowDisplayer.h"
#include "parser/file_parser.h"
#include "culling.h"
#include "render_thread.h"
#include "rz_types.h"

void renderThreadCallBack(const DrawCommand& d, void* p, Primitive* primitive, Primitive* p0, Primitive* p1);

class Dwindow : public WindowDisplayer {
public:
	Dwindow(HINSTANCE thInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow, int _width, int _height, const wchar_t* twindowName)
		: WindowDisplayer(thInstance, hPrevInstance, szCmdLine, iCmdShow, _width, _height, twindowName), width(_width), height(_height), zbuffer(_width, _height), pos(0.0f), alpha(0.0f), 
		status(DRAW_PRIMITIVE_SUBSTANCE_LINE | DRAW_PRIMITIVE_SUBSTANCE_FACE), cpuNum(6)
		/*renderThreads(cpuNum, renderThreadCallBack, this)*/ {
		ObjParser objParser;
		Mesh* mesh;
		objParser.parse("ssphere.obj", &mesh);
		float ka = 0.7;
		float kd = 0.2;
		float ks = 0.7;
		// Color(0.3, 0.56, 0.9)
		mesh->set_material(new Blinn_Phong(ka, kd, ks, new CheckerTexture(100, 100, Color(0.3, 0.56, 0.9), Color(1, 1, 1))));
		//mesh->set_material(new Blinn_Phong(0.7, 0.2, 0.2, new ConstantTexture(Color(0.3, 0.56, 0.9))));
		list = new MeshList;
		list->add(mesh);
	}
	~Dwindow() {
		
	}

	float roundFloat(float f) {
		return static_cast<int>(f + 0.5);
	}
	void setPixel(int x, int y, const RGBA& color) {
		if (x < 0 || x >= width || y < 0 || y >= height) {
			return;
		}
		WindowDisplayer::setPixel(x, y, color);
	}

	void processTriangle(const DrawCommand& d, Primitive* primitive, Primitive* p0, Primitive* p1) {
		bool makDraw = true;
		*primitive = Primitive(*d.triangle);
		for (int index = 0; index < 3; index++) {
			makDraw = true;
			computeVertex(*d.matl, *d.light, d.cam->lookfrom, primitive->vn[index], &primitive->v[index]);
			primitive->v[index].mul(*d.mvp);
			if (primitive->v[index].p.w() == 0.0f) return;
			homogenize(&primitive->v[index]);
			if (cull(primitive->v[index])) makDraw = false;
			//if (!clip(primitive, primitive->v[index], &p0, &p1)) makDraw = false;
			primitive->v[index].mul(*d.viewport);
		}
		drawPrimitive(*primitive, *d.matl, status);
	}

protected:
	MeshList* list;

	virtual RENDER_STATUS render() override {
		Color col;
		uint8_t ur, ug, ub;
		Point3f lookfrom(0.0, 0 + pos, 2.5);
		Point3f lookat(0, 0, 0);
		Vector3f vup(0, 1, 0);
		Camera cam(lookfrom, lookat, vup);
		Matrix4x4f world = rotateY(alpha);
		drawBackground();
		rasterize(*list, width, height, world, cam);
		zbuffer.resetBuffer();
		update();
		return RENDER_STATUS::CALL_STOP_SAVE_IMAGE;
	}

	float pos;
	float alpha;

	virtual void keyboardEvent(int* screen_keys) {
		if (screen_keys[VK_UP]) {
			pos += 0.1;
		}
		if (screen_keys[VK_DOWN]) {
			pos -= 0.1;
		}
		if (screen_keys[VK_LEFT]) {
			alpha += 1;
		}
		if (screen_keys[VK_RIGHT]) {
			alpha -= 1;
		}
		//rlog << "keyboardEvent\n";
	}

private:
	Color sky_color(int x, int y) {
		Color t = Color(0.7, 0.9, 1.0) - Color(0.5, 0.7, 1.0);
		Color col = Color(0.7, 0.9, 1.0) - t * (static_cast<float>(y) / height);
		return col;
	}
	inline RGBA to_rgba(const Color& col) {
		uint8_t ur, ug, ub;
		ur = col.r() > 1 ? 255 : 255.99 * col.r();
		ug = col.g() > 1 ? 255 : 255.99 * col.g();
		ub = col.b() > 1 ? 255 : 255.99 * col.b();
		return RGBA(ur, ug, ub, 255);
	}
	void drawLine(int x0, int y0, int x1, int y1, const Color& color = Color(1, 1, 1)) {
		RGBA lineColor = to_rgba(color);
		float dx = x1 - x0;
		float dy = y1 - y0;

		int sx = (dx >= 0) ? 1 : (-1);
		int sy = (dy >= 0) ? 1 : (-1);

		int x = x0;
		int y = y0;

		int isSwaped = 0;

		if (abs(dy) > abs(dx)) {
			util::swap(dx, dy);
			isSwaped = 1;
		}

		float p = 2 * (abs(dy)) - abs(dx);

		setPixel(x, y, lineColor);

		for (int i = 0; i < abs(dx); i++) {
			if (p < 0) {
				if (isSwaped == 0) {
					x = x + sx;
					setPixel(x, y, lineColor);
				}
				else {
					y = y + sy;
					setPixel(x, y, lineColor);
				}
				p += (2 * abs(dy));
			}
			else {
				x += sx;
				y += sy;
				setPixel(x, y, lineColor);
				p += (2 * abs(dy) - 2 * abs(dx));
			}
		}
	}
	bool persp_correct_interp(const Primitive& pr, const Point2f& p, Point2f* uv) const {
		Point3f b;
		barycentric(p, pr.v[0].p, pr.v[1].p, pr.v[2].p, &b);
		float alpha = b.x();
		float beta = b.y();
		float gama = b.z();
		float accurancy = 0.01f;
		if (alpha < 0.0f - accurancy || alpha > 1.0f + accurancy || beta < 0.0f - accurancy || beta > 1.0f + accurancy || gama < 0.0f - accurancy || gama > 1.0f + accurancy) {
			return false;
		}
		float u0 = pr.vt[0].x();
		float v0 = pr.vt[0].y();
		float u1 = pr.vt[1].x();
		float v1 = pr.vt[1].y();
		float u2 = pr.vt[2].x();
		float v2 = pr.vt[2].y();
		float w0 = pr.v[0].w;
		float w1 = pr.v[1].w;
		float w2 = pr.v[2].w;
		float us = alpha * (u0 / w0) + beta * (u1 / w1) + gama * (u2 / w2);
		float vs = alpha * (v0 / w0) + beta * (v1 / w1) + gama * (v2 / w2);
		float ones = alpha * (1 / w0) + beta * (1 / w1) + gama * (1 / w2);
		float u = us / ones;
		float v = vs / ones;
		*uv = Point2f(rangeFloat(0.0f, 1.0f, u), rangeFloat(0.0f, 1.0f, v));
		return true;
	}
	inline Color color_interp(const Primitive& pr, const Point2f& p) {
		Point3f b;
		barycentric(p, pr.v[0].p, pr.v[1].p, pr.v[2].p, &b);
		return pr.v[0].color * b[0] + pr.v[1].color * b[1] + pr.v[2].color * b[2];
	}
	bool persp_correct_interp(const Primitive& pr, const Point2f& p, float* z) {
		Point3f b;
		barycentric(p, pr.v[0].p, pr.v[1].p, pr.v[2].p, &b);
		float alpha = b.x();
		float beta = b.y();
		float gama = b.z();
		float accurancy = 0.01f;
		if (alpha < 0.0f - accurancy || alpha > 1.0f + accurancy || beta < 0.0f - accurancy || beta > 1.0f + accurancy || gama < 0.0f - accurancy || gama > 1.0f + accurancy) {
			return false;
		}
		float z0 = pr.v[0].p.z();
		float z1 = pr.v[1].p.z();
		float z2 = pr.v[2].p.z();
		float w0 = pr.v[0].w;
		float w1 = pr.v[1].w;
		float w2 = pr.v[2].w;
		float zs = alpha * (z0 / w0) + beta * (z1 / w1) + gama * (z2 / w2);
		float ones = alpha * (1 / w0) + beta * (1 / w1) + gama * (1 / w2);
		*z = zs / ones;
		return true;
	}
	bool pixel_color(int x, int y, const Primitive& pr, const Material& material, Color* color) {
		Color light_color = color_interp(pr, Point2f(x, y));
		Point2f uv;
		if (!persp_correct_interp(pr, Point2f(x, y), &uv)) {
			//return sky_color(x, y);
			return false;
		}
		Color texture_color = material.texture->value(uv.x(), uv.y());
		*color = light_color * texture_color;
		return true;
	}
	void setPixelInPrimitive(int x, int y, const Primitive& pr, const Material& material) {
		bool _debug_get = false;
		float z;
		persp_correct_interp(pr, Point2f(x, y), &z);
		if (!zbuffer.update(x, y, z, &mtx)) {
			// 这一点被当前点遮挡
			return;
		}
		Color color;
		if (pixel_color(x, y, pr, material, &color)) {
			setPixel(x, y, to_rgba(color));
		}
	}
	void drawScanline(int x0, int x1, int y, const Primitive& pr, const Material& material) {
		if (x0 > x1) {
			util::swap(x0, x1);
		}
		for (int x = x0; x <= x1; x++) {
			setPixelInPrimitive(x, y, pr, material);
		}
	}

	void drawLine(int x0, int y0, int x1, int y1, const Primitive& pr, const Material& material) {
		float dx = x1 - x0;
		float dy = y1 - y0;

		int sx = (dx >= 0) ? 1 : (-1);
		int sy = (dy >= 0) ? 1 : (-1);

		int x = x0;
		int y = y0;

		int isSwaped = 0;

		if (abs(dy) > abs(dx)) {
			util::swap(dx, dy);
			isSwaped = 1;
		}

		float p = 2 * (abs(dy)) - abs(dx);

		setPixelInPrimitive(x, y, pr, material);

		for (int i = 0; i < abs(dx); i++) {
			if (p < 0) {
				if (isSwaped == 0) {
					x = x + sx;
					setPixelInPrimitive(x, y, pr, material);
				}
				else {
					y = y + sy;
					setPixelInPrimitive(x, y, pr, material);
				}
				p += (2 * abs(dy));
			}
			else {
				x += sx;
				y += sy;
				setPixelInPrimitive(x, y, pr, material);
				p += (2 * abs(dy) - 2 * abs(dx));
			}
		}
	}
	void rasterize(const MeshList& list, int width, int height, const Matrix4x4f& world, const Camera& cam) {
		Point3f light = Point3f(0, 0, 2.14);
		Matrix4x4f camMatrix = cam.get_transform();

		float aspect = width / static_cast<float>(height);
		Perspective persp(90, aspect, -1, -100);
		Matrix4x4f perspMatrix = persp.get_transform();

		Viewport viewport(width, height);
		Matrix4x4f viewportMatrix = viewport.get_transform();

		Matrix4x4f m = perspMatrix * camMatrix * world;
		travelTriangle(list, m, viewportMatrix, light, cam);
	}
	void homogenize(Vertex* v) {
		v->w = v->p.w();
		v->p /= v->p.w();
	}
	void computeVertex(const Material& material, const Point3f& light, const Point3f& eye, const Vector3f& normal, Vertex* v) {
		v->color = material.light_value(*v, light, eye, normal);
	}
	void travelTriangle(const MeshList& list, const Matrix4x4f& mvp, const Matrix4x4f& viewport, const Point3f& light, const Camera& cam) {
		DrawCommand command(DrawCommandEnumClass::DRAW_PRIMITIVE, nullptr, &mvp, &viewport, &light, nullptr, &cam);
		//int thread_index = 0;
		//
		//for (const Mesh* mesh : list) {
		//	command.matl = mesh->ptr_mat;
		//	thread_index = 0;
		//	for (const Triangle* tg : *mesh) {
		//		command.triangle = tg;
		//		renderThreads.commit(thread_index, command);
		//		thread_index++;
		//		if (thread_index == cpuNum) thread_index = 0;
		//	}
		//}
		//
		//for (thread_index = 0; thread_index < cpuNum; thread_index++) {
		//	renderThreads.commit(thread_index, DrawCommand(DrawCommandEnumClass::DRAW_PAUSE));
		//}

		//renderThreads.waitForAllDone();

		for (const Mesh* mesh : list) {
			for (const Triangle* tg : *mesh) {
				bool makDraw = true;
				primitive = Primitive(*tg);
				for (int index = 0; index < 3; index++) {
					makDraw = true;
					computeVertex(*mesh->ptr_mat, light, cam.lookfrom, primitive.vn[index], &primitive.v[index]);
					primitive.v[index].mul(mvp);
					if (primitive.v[index].p.w() == 0.0f) return;
					homogenize(&primitive.v[index]);
					if (cull(primitive.v[index])) makDraw = false;
					//if (!clip(primitive, primitive->v[index], &p0, &p1)) makDraw = false;
					primitive.v[index].mul(viewport);
					primitive.v[index].p.rx() = round(primitive.v[index].p.x());
					primitive.v[index].p.ry() = round(primitive.v[index].p.y());
				}
				drawPrimitive(primitive, *mesh->ptr_mat, status);
			}
		}
	}
	void sortDescendingByY(const Vertex& _v0, const Vertex& _v1, const Vertex& _v2, Vertex const ** v0, Vertex const ** v1, Vertex const ** v2) {
		if (_v0.p.y() > _v1.p.y()) {
			if (_v0.p.y() > _v2.p.y()) {
				if (_v1.p.y() > _v2.p.y()) {
					*v0 = &_v0;
					*v1 = &_v1;
					*v2 = &_v2;
				}
				else {
					*v0 = &_v0;
					*v1 = &_v2;
					*v2 = &_v1;
				}
			}
			else {
				*v0 = &_v2;
				*v1 = &_v0;
				*v2 = &_v1;
			}
		}
		else {
			if (_v0.p.y() < _v2.p.y()) {
				if (_v1.p.y() < _v2.p.y()) {
					*v0 = &_v2;
					*v1 = &_v1;
					*v2 = &_v0;
				}
				else {
					*v0 = &_v1;
					*v1 = &_v2;
					*v2 = &_v0;
				}
			}
			else {
				*v0 = &_v1;
				*v1 = &_v0;
				*v2 = &_v2;
			}
		}
	}
	void fillBottomFlatTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Primitive& pr, const Material& material) {
		float invslope0 = (v1.p.x() - v0.p.x()) / (v1.p.y() - v0.p.y());
		float invslope1 = (v2.p.x() - v0.p.x()) / (v2.p.y() - v0.p.y());
		float curx0 = v0.p.x();
		float curx1 = v0.p.x();
		for (int scanlineY = v0.p.y(); scanlineY >= v1.p.y(); scanlineY--) {
			drawScanline(curx0, curx1, scanlineY, pr, material);
			curx0 -= invslope0;
			curx1 = std::ceil(curx1 - invslope1);
		}
	}
	void fillTopFlatTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Primitive& pr, const Material& material) {
		float invslope0 = (v2.p.x() - v0.p.x()) / (v2.p.y() - v0.p.y());
		float invslope1 = (v2.p.x() - v1.p.x()) / (v2.p.y() - v1.p.y());

		float curx0 = v2.p.x();
		float curx1 = v2.p.x();

		for (int scanlineY = v2.p.y(); scanlineY <= v0.p.y(); scanlineY++) {
			drawScanline(curx0, curx1, scanlineY, pr, material);
			curx0 = std::ceil(curx0 + invslope0);
			curx1 += invslope1;
		}
	}
	void drawPrimitive(const Primitive& pr, const Material& material, DRAW_PRIMITIVE_SUBSTANCE_STATUS status) {
		if (status & DRAW_PRIMITIVE_SUBSTANCE_VERTEX) {
			setPixel(pr.v[0].p.x(), pr.v[0].p.y(), to_rgba(Color(1, 1, 1)));
			setPixel(pr.v[1].p.x(), pr.v[1].p.y(), to_rgba(Color(1, 1, 1)));
			setPixel(pr.v[2].p.x(), pr.v[2].p.y(), to_rgba(Color(1, 1, 1)));
		}
		if (status & DRAW_PRIMITIVE_SUBSTANCE_LINE) {
			drawLine(pr.v[0].p.x(), pr.v[0].p.y(), pr.v[1].p.x(), pr.v[1].p.y(), Color(0.95, 0.4, 0.23));
			drawLine(pr.v[1].p.x(), pr.v[1].p.y(), pr.v[2].p.x(), pr.v[2].p.y(), Color(0.95, 0.4, 0.23));
			drawLine(pr.v[2].p.x(), pr.v[2].p.y(), pr.v[0].p.x(), pr.v[0].p.y(), Color(0.95, 0.4, 0.23));
		}
		if (status & DRAW_PRIMITIVE_SUBSTANCE_FACE) {
			const Vertex* v0, * v1, * v2;
			sortDescendingByY(pr.v[0], pr.v[1], pr.v[2], &v0, &v1, &v2);

			// v0->p.y >= v1->p.y >= v2->p.y
			if (v1->p.y() == v2->p.y()) {
				fillBottomFlatTriangle(*v0, *v1, *v2, pr, material);
			}
			else if (v0->p.y() == v1->p.y()) {
				fillTopFlatTriangle(*v0, *v1, *v2, pr, material);
			}
			else {
				Vertex vnew(Point3f(v0->p.x() + ((v1->p.y() - v0->p.y()) / (v2->p.y() - v0->p.y())) * (v2->p.x() - v0->p.x()), v1->p.y()));
				fillBottomFlatTriangle(*v0, *v1, vnew, pr, material);
				fillTopFlatTriangle(*v1, vnew, *v2, pr, material);
			}
		}
	}

	void drawBackground() {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				setPixel(x, y, to_rgba(sky_color(x, y)));
			}
		}
	}

	int width;
	int height;
	ZBuffer zbuffer;
	DRAW_PRIMITIVE_SUBSTANCE_STATUS status;
	const int cpuNum;
	//RenderThread renderThreads;
	std::mutex mtx;
	Primitive primitive;
	Primitive p0;
	Primitive p1;
};

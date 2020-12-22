#pragma once
#include "display/WindowDisplayer.h"
#include "color.h"
#include "parser/file_parser.h"
#include "camera.h"

class Dwindow : public WindowDisplayer {
public:
	Dwindow(HINSTANCE thInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow, int _width, int _height, const wchar_t* twindowName)
		: WindowDisplayer(thInstance, hPrevInstance, szCmdLine, iCmdShow, _width, _height, twindowName), width(_width), height(_height) {}
	~Dwindow() {}

protected:
	virtual RENDER_STATUS render() override {
		Color col;
		uint8_t ur, ug, ub;
		ObjParser objParser;
		Mesh* mesh;
		objParser.parse("triangle2.obj", &mesh);
		MeshList* list = new MeshList;
		list->add(mesh);
		rasterize(list, width, height);
		//for (int x = 0; x < width; x++ ) {
		//	for (int y = 0; y < height; y++) {
		//		col = sky_color(x, y);
		//		setPixel(x, y, to_rgba(col));
		//	}
		//}

		update();
		return RENDER_STATUS::CALL_STOP;
	}

private:
	Color sky_color(int x, int y) {
		Color t = Color(0.7, 0.9, 1.0) - Color(0.5, 0.7, 1.0);
		Color col = Color(0.7, 0.9, 1.0) - t * (static_cast<float>(y) / height);
		return col;
	}
	RGBA to_rgba(const Color& col) {
		uint8_t ur, ug, ub;
		ur = 255.99 * col.r();
		ug = 255.99 * col.g();
		ub = 255.99 * col.b();
		return RGBA(ur, ug, ub, 255);
	}
	void draw_vertex(const Vertex& v) {
		rlog.print("draw %d,%d\n", static_cast<int>(v.p.x()), static_cast<int>(v.p.y()));
		setPixel(v.p.x(), v.p.y(), RGBA(255, 255, 255, 255));
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
	void drawLine(const Vertex& v0, const Vertex& v1) {
		int x0 = v0.p.x();
		int x1 = v1.p.x();
		int y0 = v0.p.y();
		int y1 = v1.p.y();
		drawLine(x0, y0, x1, y1);
	}
	void rasterize(MeshList* list, int width, int height) {
		Point3f lookfrom(0, 0, 1);
		Point3f lookat(0, 0, 0);
		Vector3f vup(0, 1, 0);
		Camera cam(lookfrom, lookat, vup);
		Matrix4x4f camMatrix = cam.get_transform();

		float aspect = width / static_cast<float>(height);
		Perspective persp(90, aspect, -1, -100);
		Matrix4x4f perspMatrix = persp.get_transform();

		Viewport viewport(width, height);
		Matrix4x4f viewportMatrix = viewport.get_transform();

		Matrix4x4f m = viewportMatrix * perspMatrix * camMatrix;
		travelTriangle(list, m);
	}
	bool normalize(Vertex* v) {
		if (v->p.w() == 0) {
			return false;
		}
		v->p /= v->p.w();
		return true;
	}
	bool normalize(Triangle* tg) {
		if (!normalize(tg->v[0])) return false;
		if (!normalize(tg->v[1])) return false;
		if (!normalize(tg->v[2])) return false;
		return true;
	}
	void travelTriangle(MeshList* list, const Matrix4x4f& m) {
		float w0, w1, w2;
		w0 = w1 = w2 = 1;
		for (Mesh* mesh : *list) {
			for (Triangle* tg : *mesh) {
				tg->mul(m);
				w0 = tg->v[0]->p.w();
				w1 = tg->v[1]->p.w();
				w2 = tg->v[2]->p.w();
				normalize(tg);
				drawLine(*tg->v[0], *tg->v[1]);
				drawLine(*tg->v[1], *tg->v[2]);
				drawLine(*tg->v[2], *tg->v[0]);
				drawTriangle(*tg);
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
	void fillBottomFlatTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
		float invslope0 = (v1.p.x() - v0.p.x()) / (v1.p.y() - v0.p.y());
		float invslope1 = (v2.p.x() - v0.p.x()) / (v2.p.y() - v0.p.y());

		float curx0 = v0.p.x();
		float curx1 = v0.p.x();

		for (int scanlineY = v0.p.y(); scanlineY >= v1.p.y(); scanlineY--) {
			drawLine(curx0, scanlineY, curx1, scanlineY);
			curx0 -= invslope0;
			curx1 -= invslope1;
		}
	}
	void fillTopFlatTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
		float invslope0 = (v2.p.x() - v0.p.x()) / (v2.p.y() - v0.p.y());
		float invslope1 = (v2.p.x() - v1.p.x()) / (v2.p.y() - v1.p.y());

		float curx0 = v2.p.x();
		float curx1 = v2.p.x();

		for (int scanlineY = v2.p.y(); scanlineY <= v0.p.y(); scanlineY++) {
			drawLine(curx0, scanlineY, curx1, scanlineY);
			curx0 += invslope0;
			curx1 += invslope1;
		}
	}
	void drawTriangle(const Triangle& tg) {
		const Vertex* v0, * v1, * v2;
		sortDescendingByY(*tg.v[0], *tg.v[1], *tg.v[2], &v0, &v1, &v2);
		
		// v0->p.y >= v1->p.y >= v2->p.y
		if (v1->p.y() == v2->p.y()) {
			fillBottomFlatTriangle(*v0, *v1, *v2);
		}
		else if (v0->p.y() == v1->p.y()) {
			fillTopFlatTriangle(*v0, *v1, *v2);
		}
		else {
			Vertex vnew(Point3f(v0->p.x() + ((v1->p.y() - v0->p.y()) / (v2->p.y() - v0->p.y())) * (v2->p.x() - v0->p.x()), v1->p.y()));
			fillBottomFlatTriangle(*v0, *v1, vnew);
			fillTopFlatTriangle(*v1, vnew, *v2);
		}
	}

	int width, height;
};

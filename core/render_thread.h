#pragma once
#include <thread>
#include <queue>
#include <map>
#include "mesh.h"
#include "material.h"
#include "camera.h"

enum class DrawCommandEnumClass {
	DRAW_PRIMITIVE,
	DRAW_PAUSE,
	DRAW_STOP
};

enum class RunningState {
	RUNNING,
	STOPPED
};

class DrawCommand {
public:
	DrawCommand() {}
	DrawCommand(const DrawCommandEnumClass& _command) : 
		command(_command) {}
	DrawCommand(const DrawCommandEnumClass& _command, const Triangle* tg, const Matrix4x4f* _mvp, const Matrix4x4f* _viewport, const Point3f* _light, const Material* _matl, const Camera* _cam) : 
		command(_command), triangle(tg), mvp(_mvp), viewport(_viewport), light(_light), matl(_matl), cam(_cam) {}
	~DrawCommand() {}

	DrawCommandEnumClass command;
	const Triangle* triangle;
	const Matrix4x4f* mvp;
	const Matrix4x4f* viewport;
	const Point3f* light;
	const Camera* cam;
	const Material* matl;
};

typedef std::queue<DrawCommand> DrawQueue;

class RenderThread {
public:
	RenderThread(int _cpuNum, void(*fn)(const DrawCommand&, void*, Primitive* primitive, Primitive* p0, Primitive* p1), void* p) :
		cpuNum(_cpuNum), drawFn(fn) {
		threads = std::shared_ptr<std::thread>(new std::thread[_cpuNum], std::default_delete<std::thread[]>());
		drawQueue = std::shared_ptr<DrawQueue>(new DrawQueue[_cpuNum], std::default_delete<DrawQueue[]>());
		states = std::shared_ptr<RunningState>(new RunningState[_cpuNum], std::default_delete<RunningState[]>());
		for (int index = 0; index < _cpuNum; index++) {
			threads.get()[index] = std::thread([=]() {
				DrawCommand command;
				states.get()[index] = RunningState::STOPPED;
				Primitive primitive;
				Primitive p0, p1;
				while (true) {
					if (!drawQueue.get()[index].empty()) {
						states.get()[index] = RunningState::RUNNING;
						command = drawQueue.get()[index].front();
						if (command.command == DrawCommandEnumClass::DRAW_STOP) {
							break;
						}
						if (command.command == DrawCommandEnumClass::DRAW_PAUSE) {
							states.get()[index] = RunningState::STOPPED;
							drawQueue.get()[index].pop();
							continue;
						}
						fn(command, p, &primitive, &p0, &p1);
						drawQueue.get()[index].pop();
					}
				}
				states.get()[index] = RunningState::STOPPED;
			});
		}
	}
	~RenderThread() {
		commitStopCommandAll();
		join();
	}

	void commit(int thread_index, const DrawCommand& d) {
		drawQueue.get()[thread_index].push(d);
	}
	void commitStopCommandAll() {
		for (int index = 0; index < cpuNum; index++) {
			drawQueue.get()[index].push(DrawCommandEnumClass::DRAW_STOP);
		}
	}
	void join() {
		for (int index = 0; index < cpuNum; index++) {
			threads.get()[index].join();
		}
	}
	inline bool allDone() {
		for (int index = 0; index < cpuNum; index++) {
			if (states.get()[index] == RunningState::RUNNING) return false;
		}
		return true;
	}
	void waitForAllDone() {
		while (!allDone()) {
			;
		}
	}

	std::shared_ptr<std::thread> threads;
	std::shared_ptr<DrawQueue> drawQueue;
	std::shared_ptr<RunningState> states;
	void(*drawFn)(const DrawCommand&, void* p, Primitive* primitive, Primitive* p0, Primitive* p1);
	const int cpuNum;
};
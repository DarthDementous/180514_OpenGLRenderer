#pragma once

struct GLFWwindow;

class Program {
public:
	Program();
	virtual ~Program();

	int Run(const char* a_windowName, int a_width, int a_height);
protected:
	virtual int	 Startup()	= 0;
	virtual void Shutdown() = 0;

	virtual void Update(float a_dt) = 0;
	virtual void Render() = 0;
private:
	GLFWwindow* InitialiseWindow(const char* a_windowName, int a_width, int a_height);
	void DestroyContextWindow();
};
#pragma once

#include <gl_core_4_4.h>

class Program {
public:
	Program();
	virtual ~Program();

	int Run();
protected:
	virtual int	 Startup()	= 0;
	virtual void Shutdown() = 0;

	virtual void Update(float a_dt) = 0;
	virtual void Render() = 0;
private:
};
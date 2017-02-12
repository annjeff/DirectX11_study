#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

class Input
{
public:
	Input();
	
	explicit Input(const Input&);
	
	~Input();

public:
	void Initialize();

	void KeyDown(unsigned int);
	
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_keys[256];
};

#endif
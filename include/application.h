#ifndef SCOP_APPLICATION_H
#define SCOP_APPLICATION_H


#ifndef SCOP_WINDOW_TITLE
#define SCOP_WINDOW_TITLE "scop"
#endif

#ifndef SCOP_WINDOW_WIDTH
#define SCOP_WINDOW_WIDTH 800
#endif

#ifndef SCOP_WINDOW_HEIGHT
#define SCOP_WINDOW_HEIGHT 600
#endif

class Application {
public:
	Application() = delete;
	Application(const Application &) = delete;
	Application &operator=(const Application &) = delete;

	Application(int ac, char **av);

	int run();
};

#endif //SCOP_APPLICATION_H

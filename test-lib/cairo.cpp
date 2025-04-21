
#include <cairomm/context.h>
#include <iostream>

int main() {
	Cairo::RefPtr<Cairo::ImageSurface> surface =
		Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 300, 300);

	Cairo::RefPtr<Cairo::Context> cr =
		Cairo::Context::create(surface);

	surface->write_to_png("cairo.png");

	std::cout << "Hello, World!"
			  << std::endl;
}

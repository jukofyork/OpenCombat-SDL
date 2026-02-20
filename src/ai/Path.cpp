#include "./Path.h"

// XXX/GWS: Both of these need to use a memory pool allocator!!!
Path *
AllocatePath()
{
	return new Path();
}

void
FreePath(Path *path, bool recurse)
{
	if(nullptr == path) return;

	if(recurse) {
		Path *t = nullptr;
		while(path != nullptr) {
			t = path->Next;
			delete path;
			path = t;
		}
	} else {
		delete path;
	}
}

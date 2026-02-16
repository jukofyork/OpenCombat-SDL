#include "./Animation.h"
#include <graphics/Screen.h>

Animation::Animation(const std::string &name)
{
	_reverse = false;
	_name = name;
	for(int i = 0; i < NumDirections; ++i) {
		_currentFrameNums[i] = 0;
		_totalTimes[i] = 0;
		_incrementalTimes[i] = 0;
	}
}

Animation::~Animation(void)
{
}

void
Animation::AddFrame(Frame *f, Direction dir)
{
	_frames[dir].push_back(std::unique_ptr<Frame>(f));
}

Animation *
Animation::Clone()
{
	Animation *a = new Animation(_name);
	a->_reverse = _reverse;
	for(int dir = 0; dir < NumDirections; ++dir) {
		for(auto& frame : _frames[dir]) {
			a->_frames[dir].push_back(std::unique_ptr<Frame>(frame->Clone()));
		}
	}

	for(int i = 0; i < NumDirections; ++i) {
		a->_incrementalTimes[i] = _incrementalTimes[i];
		a->_totalTimes[i] = _totalTimes[i];
	}
	return a;
}

void
Animation::Render(Screen *screen, Direction heading, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx)
{
	if(_frames[heading].empty()) return;
	_frames[heading][_currentFrameNums[heading]]->Render(screen, x, y, hilit, hilitColor, camouflageIdx);
}


void
Animation::Update(long dt)
{
	for(int heading = 0; heading < NumDirections; ++ heading)
	{
		if(_frames[heading].empty()) continue;

		_incrementalTimes[heading] += dt;
		_totalTimes[heading] += dt;

		if(_totalTimes[heading] >= _frames[heading][_currentFrameNums[heading]]->GetDisplayTime()) {
			// Update the timers
			_totalTimes[heading] = 0;
			_incrementalTimes[heading] = 0;
			// Increment the current frame number
			if(_reverse)
			{
				_currentFrameNums[heading] = (_currentFrameNums[heading]-1);
				if(_currentFrameNums[heading] < 0)
				{
					_currentFrameNums[heading] += static_cast<int>(_frames[heading].size());
				}
			}
			else
			{
				_currentFrameNums[heading] = (_currentFrameNums[heading]+1) % static_cast<int>(_frames[heading].size());
			}
		}
	}
}

void
Animation::GetExtents(Direction heading, int x, int y, Region *r)
{
	if (_frames[heading].empty()) {
		// Safety check - no frames loaded
		r->points[0].x = r->points[0].y = 0;
		r->points[1].x = r->points[1].y = 0;
		r->points[2].x = r->points[2].y = 0;
		r->points[3].x = r->points[3].y = 0;
		return;
	}
	_frames[heading][_currentFrameNums[heading]]->GetExtents(x, y, r);
}

int
Animation::GetCurrentFrameNumber(Direction heading)
{
	return _currentFrameNums[heading];
}

void
Animation::Reset()
{
	for(int i = 0; i < NumDirections; ++i) {
		_totalTimes[i] = 0;
		_incrementalTimes[i] = 0;
		if(_reverse && !_frames[i].empty())
		{
			_currentFrameNums[i] = static_cast<int>(_frames[i].size())-1;
		}
		else
		{
			_currentFrameNums[i] = 0;
		}
	}
}

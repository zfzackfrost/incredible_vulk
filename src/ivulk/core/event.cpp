#include <ivulk/core/event.hpp>

#include <queue>
#include <unordered_map>

namespace ivulk {
	struct Instance
	{
		std::queue<Event> events;
		std::unordered_map<E_EventType, std::vector<EventManager::Callback>> callbacks;
	};

	Instance s_inst;

	void EventManager::addCallback(E_EventType eventType, const Callback& cb)
	{
		auto& cbArray = s_inst.callbacks[eventType];
		cbArray.push_back(cb);
	}

	void EventManager::pushEvent(const Event evt) { s_inst.events.push(evt); }

	void EventManager::pushEvent(const SDL_Event sdlEvt)
	{
		if (auto evt = Event::fromSDLEvent(sdlEvt))
			pushEvent(*evt);
	}

	void EventManager::processAllEvents()
	{
		while (!s_inst.events.empty())
		{
			popEvent();
		}
	}

	void EventManager::popEvent()
	{
		auto evt = s_inst.events.front();
		s_inst.events.pop();
		for (const auto& cb : s_inst.callbacks[evt.type])
		{
			cb(evt);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	//                                    Event                                    //
	/////////////////////////////////////////////////////////////////////////////////

	std::optional<Event> Event::fromSDLEvent(SDL_Event evt)
	{
		E_EventType type;
		AnyEventData data;

		if (evt.type == SDL_KEYUP)
		{
			return makeKeyEvent({
				.bRepeat = !!evt.key.repeat,
				.bIsDown = false,
				.keycode = evt.key.keysym.sym,
			});
		}
		if (evt.type == SDL_KEYDOWN)
		{
			return makeKeyEvent({
				.bRepeat = !!evt.key.repeat,
				.bIsDown = true,
				.keycode = evt.key.keysym.sym,
			});
		}
		if (evt.type == SDL_MOUSEBUTTONUP)
		{
			return makeMouseBtnEvent({
				.bIsDown = false,
				.button  = evt.button.button,
			});
		}
		if (evt.type == SDL_MOUSEBUTTONDOWN)
		{
			return makeMouseBtnEvent({
				.bIsDown = true,
				.button  = evt.button.button,
			});
		}
		if (evt.type == SDL_MOUSEMOTION)
		{
			return makeMouseMoveEvent({
				.mousePos   = {evt.motion.x, evt.motion.y},
				.mouseDelta = {evt.motion.xrel, evt.motion.yrel},
			});
		}

		return {};
	}
} // namespace ivulk

#pragma once

#include <map>

#include <Event.h>
#include <Thread.h>
#include <window.h>
#include <CriticalSection.h>

class Notifier
{
protected:
	class Target
	{
	public:
		Target(const Window* p) : m_window(p), m_thread(NULL), m_event(NULL) {}
		Target(const Thread* t, Event* ev) : m_window(NULL), m_thread(t), m_event(ev) {}

		const Window* GetWindow() const {	return m_window;	}
		const Thread* GetThread() const {	return m_thread;	}

		void Notify(bool isUI, const AppMessage& msg, WPARAM wParam, LPARAM lParam) {
			if (m_window) {
				if (isUI) {
					SendMessage(*m_window, msg, wParam, lParam);
				}
				else {
					PostMessage(*m_window, msg, wParam, lParam);
				}
			}
			else if (m_event) {
				m_event->Set();
			}
		}

	protected:
		const Window* m_window; // may be NULL
		const Thread* m_thread;
		Event* m_event;
	};

public:
	void AddNotifyTarget(const Window* p, const std::vector<AppMessage>& ams) {
		for (auto& am : ams) {
			AddNotifyTarget(p, am);
		}
	}

	void AddNotifyTarget(const Window* p, const AppMessage& am) {
		CSLocker csl(m_csMap);
		auto it = m_mapTargets.find(am);
		if (it == m_mapTargets.end()) {
			std::vector<Target> targets;
			targets.push_back(Target(p));
			m_mapTargets[am] = targets;
		}
		else {
			it->second.push_back(Target(p));
		}
	}

	void AddNotifyTarget(const Thread* t, Event* ev, const std::vector<AppMessage>& ams) {
		for (auto& am : ams) {
			AddNotifyTarget(t, ev, am);
		}
	}

	void AddNotifyTarget(const Thread* t, Event* ev, const AppMessage& am) {
		CSLocker csl(m_csMap);
		auto it = m_mapTargets.find(am);
		if (it == m_mapTargets.end()) {
			std::vector<Target> targets;
			targets.push_back(Target(t, ev));
			m_mapTargets[am] = targets;
		}
		else {
			it->second.push_back(Target(t, ev));
		}
	}

	void Notify(Window* p, const AppMessage& msg, WPARAM wParam = 0, LPARAM lParam = 0) {
		CSLocker csl(m_csMap);
		auto it = m_mapTargets.find(msg);
		if (it == m_mapTargets.end())
			return;

		for (auto& target : it->second) {
			if (target.GetWindow() != p) {
				target.Notify(true, msg, wParam, lParam);
			}
		}
	}

	void Notify(Thread* t, const AppMessage& msg, WPARAM wParam = 0, LPARAM lParam = 0) {
		CSLocker csl(m_csMap);
		auto it = m_mapTargets.find(msg);
		if (it == m_mapTargets.end())
			return;

		for (auto& target : it->second) {
			if (target.GetThread() != t) {
				target.Notify(false, msg, wParam, lParam);
			}
		}
	}

	void Notify(const AppMessage& msg, WPARAM wParam = 0, LPARAM lParam = 0) {
		CSLocker csl(m_csMap);
		auto it = m_mapTargets.find(msg);
		if (it == m_mapTargets.end())
			return;

		for (auto& target : it->second) {
			target.Notify(false, msg, wParam, lParam);
		}
	}
protected:
	CriticalSection m_csMap;
	std::map<AppMessage, std::vector<Target>> m_mapTargets;
};


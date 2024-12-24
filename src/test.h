#include <iostream>
#include <vector>

namespace mylib {
// private base class
template<class T>
class observerBase {
  private:
	virtual bool shouldNotify(const std::string &key) = 0;
	virtual void update(T *sender, const std::string &key) = 0;

	template<class F>
	friend class observable;
};

// observer
template<class T, const std::string &...keys>
class observer : public observerBase<T> {
  public:
	virtual void update(T *sender, const std::string &key) = 0;

  private:
	std::vector<std::string> observeKeys = {keys...};
	bool shouldNotify(const std::string &key) {
		return (std::find(observeKeys.begin(), observeKeys.end(), key) != observeKeys.end());
	}
};

// observable
template<class T>
class observable {
  public:
	void addObserver(observerBase<T> *observer) {
		m_observers.push_back(observer);
	}

	void removeObserver(observerBase<T> *observer) {
		auto it = std::find(m_observers.begin(), m_observers.end(), observer);
		if (it != m_observers.end())
			m_observers.erase(it);
	}

	void notify(const std::string &key) {
		for (const auto &o: m_observers) {
			if (o->shouldNotify(key))
				o->update(static_cast<T *>(this), key);
		}
	}

  private:
	std::vector<observerBase<T> *> m_observers;
};
}// namespace mylib

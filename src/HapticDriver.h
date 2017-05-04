#pragma once

class EventRequest;
class RTPRequest;


class HapticDriver {
public:

	virtual void visit(const EventRequest&) {}
	virtual void visit(const RTPRequest&) {}

	virtual bool canVisit(const EventRequest&) { return false; }
	virtual bool canVisit(const RTPRequest&) { return false; }
};



class RTPDriver : public HapticDriver {
private:
	void visit(const RTPRequest& req) override;
	bool canVisit(const RTPRequest& req) override;
};

class EventDriver : public HapticDriver {
private:
	void visit(const EventRequest& req) override;
	bool canVisit(const EventRequest& req) override { return true; }
};
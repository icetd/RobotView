#ifndef NODE_LAYER_H
#define NODE_LAYER_H

#include "../Core/Layer.h"
#include "../Ros/AppNode.h"
#include "../Utils/INIReader.h"

class NodeLayer : public Layer
{
protected:
  virtual void OnAttach() override;
  virtual void OnUpdate(float ts) override;
  virtual void OnDetach() override;
  virtual void OnUIRender() override;

private:
	std::string m_master_url;
	AppNode* m_AppNode = nullptr;

    void ShowNodeControl();

    INIReader *m_config;
};

#endif
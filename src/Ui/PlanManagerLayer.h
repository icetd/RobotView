#ifndef NAVPLAN_LAYER_H
#define NAVPLAN_LAYER_H

#include "../Core/Layer.h"
#include "../Core/Scene.h"
#include "../Graphics/Buffers/VertexBuffer.h"
#include "../Plan/NavPlan.h"
#include "../Plan/PlanManager.h"
#include <functional>
#include <memory>

class PlanManagerLayer : public Layer
{
public:
   static void SetCurrentGoal(int goal_id) {
    m_current_goal = goal_id;
  }

protected:
  virtual void OnAttach() override;
  virtual void OnUpdate(float ts) override;
  virtual void OnDetach() override;
  virtual void OnUIRender() override;
private:
  std::unique_ptr<PlanManager> m_PlanManager;
  int m_current_plan;  // index of (plan_id - 1)
  static int m_current_goal;
  void ShowNavPlan();
  
  std::unique_ptr<NavPlan> m_make_plan;
  int m_make_goal_num;
  int m_make_plan_num;
  void ShowMakePlan();

  bool m_isMakePlanMode;
  std::vector<Vertex> m_current_plan_vertex;
  void UpdatePlan();
};

#endif
#ifndef PARS_VIEWER_VIEWER_HPP
#define PARS_VIEWER_VIEWER_HPP

#include <cppzmq/zmq.hpp>
#include <QMainWindow>
#include <QTimer>
#include <QWidget>

#include <pars_viewer/transform.hpp>
#include <ui_viewer.h>

namespace pars
{
class viewer : public QMainWindow, public Ui_window, public zmq::monitor_t
{
  Q_OBJECT

public:
  explicit viewer(QWidget* parent = nullptr);

protected:
  void on_event_connected               (const zmq_event_t& event, const char* address) override;
  void on_event_disconnected            (const zmq_event_t& event, const char* address) override;
  void on_event_accepted                (const zmq_event_t& event, const char* address) override;
  void on_event_closed                  (const zmq_event_t& event, const char* address) override;

  void initialize_connection            ();
  void finalize_connection              ();
  void tick                             ();
  void update_parameters                ();

  void set_connection_widgets_enabled   (const bool enabled);
  void set_configuration_widgets_enabled(const bool enabled);
  
  void keyPressEvent                    (QKeyEvent*   key_event  ) override;
  void keyReleaseEvent                  (QKeyEvent*   key_event  ) override;
  void mousePressEvent                  (QMouseEvent* mouse_event) override;
  void mouseMoveEvent                   (QMouseEvent* mouse_event) override;

  QTimer                         timer_    ;
  zmq::context_t                 context_  ;
  std::unique_ptr<zmq::socket_t> socket_   ;
  transform                      transform_;
  
  QPoint                         position_   ;
  float                          move_speed_ = 1.0f;
  float                          look_speed_ = 0.1f;
};
}

#endif
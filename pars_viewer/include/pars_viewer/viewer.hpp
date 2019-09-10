#ifndef PARS_VIEWER_VIEWER_HPP
#define PARS_VIEWER_VIEWER_HPP

#include <cppzmq/zmq.hpp>
#include <QMainWindow>
#include <QTimer>
#include <QWidget>

#include <ui_viewer.h>

namespace pars
{
class viewer : public QMainWindow, public Ui_window, public zmq::monitor_t
{
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

  QTimer                         timer_  ;
  zmq::context_t                 context_;
  std::unique_ptr<zmq::socket_t> socket_ ;
};
}

#endif
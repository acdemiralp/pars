#include <pars_viewer/viewer.hpp>

#include <image.pb.h>
#include <settings.pb.h>

namespace pars
{
viewer::viewer(QWidget* parent) : QMainWindow(parent), timer_(this), context_(1)
{
  setupUi      (this);
  showMaximized();
  
  QObject::connect(button_connect, &QPushButton::clicked, [&] () 
  {
    socket_ ? finalize_connection() : initialize_connection();
  });
  
  QObject::connect(button_update , &QPushButton::clicked, [&] () 
  {
    update_parameters();
  });

  QObject::connect(&timer_       , &QTimer::timeout     , [&] ()
  {
    tick();
  });
  timer_.start(16);
}

void viewer::on_event_connected               (const zmq_event_t& event, const char* address)
{
  set_configuration_widgets_enabled(true);
}
void viewer::on_event_disconnected            (const zmq_event_t& event, const char* address)
{
  finalize_connection();
}
void viewer::on_event_accepted                (const zmq_event_t& event, const char* address)
{
  set_configuration_widgets_enabled(true);
}
void viewer::on_event_closed                  (const zmq_event_t& event, const char* address)
{
  finalize_connection();
}

void viewer::initialize_connection            ()
{
  const auto ip      = text_ip  ->text().toStdString();
  const auto port    = text_port->text().toStdString();
  const auto address = std::string("tcp://") + ip + ":" + port;

  abort();
  socket_ = std::make_unique<zmq::socket_t>(context_, ZMQ_PAIR);
  socket_->setsockopt(ZMQ_LINGER, 0);
  init (*socket_, "inproc://monitor");
  socket_->connect(address);

  set_connection_widgets_enabled   (false);
}
void viewer::finalize_connection              ()
{
  abort();
  socket_.reset();

  set_connection_widgets_enabled   (true );
  set_configuration_widgets_enabled(false);
}
void viewer::tick                             ()
{
  if (socket_)
    check_event();
}
void viewer::update_parameters                ()
{
  button_update->setEnabled(false);

  std::string mode;
  if (checkbox_particle_advection_enabled->isChecked()) mode += "streamlines ";
  if (checkbox_volume_rendering_enabled  ->isChecked()) mode += "volume "     ;

  settings settings;
  settings.set_mode                           (mode);
  settings.set_volume_type                    (combobox_scalars->currentText().toStdString());
  settings.set_dataset_filepath               (text_filepath->text().toStdString());
  settings.add_seed_generation_stride         (text_stride_x->text().toInt());
  settings.add_seed_generation_stride         (text_stride_y->text().toInt());
  settings.add_seed_generation_stride         (text_stride_z->text().toInt());
  settings.set_seed_generation_iterations     (text_particle_advection_iterations->text().toInt());
  settings.set_particle_tracing_integrator    (combobox_integrator->currentText().toStdString());
  settings.set_particle_tracing_step_size     (text_step_size->text().toFloat());
  settings.set_particle_tracing_load_balance  (checkbox_load_balancing_enabled->isChecked());
  settings.set_color_generation_mode          (combobox_mode->currentText().toStdString());
  settings.set_color_generation_free_parameter(text_parameter->text().toFloat());
  settings.add_raytracing_camera_position     (text_position_x->text().toFloat());
  settings.add_raytracing_camera_position     (text_position_y->text().toFloat());
  settings.add_raytracing_camera_position     (text_position_z->text().toFloat());
  settings.add_raytracing_camera_forward      (text_forward_x->text().toFloat());
  settings.add_raytracing_camera_forward      (text_forward_y->text().toFloat());
  settings.add_raytracing_camera_forward      (text_forward_z->text().toFloat());
  settings.add_raytracing_camera_up           (text_up_x->text().toFloat());
  settings.add_raytracing_camera_up           (text_up_y->text().toFloat());
  settings.add_raytracing_camera_up           (text_up_z->text().toFloat());
  settings.add_raytracing_image_size          (label_image->width());
  settings.add_raytracing_image_size          (label_image->height());
  settings.set_raytracing_streamline_radius   (text_line_radius->text().toFloat());
  settings.set_raytracing_iterations          (text_ray_tracing_iterations->text().toInt());
  
  std::string settings_string;
  settings.SerializeToString(&settings_string);

  zmq::message_t request(settings_string.size());
  memcpy(request.data(), settings_string.data(), settings_string.size());
  socket_->send(request);

  zmq::message_t response;
  socket_->recv(&response);

  pars::image image;
  image.ParseFromArray(response.data(), static_cast<std::int32_t>(response.size()));

  QPixmap pixmap;
  pixmap.convertFromImage(QImage(reinterpret_cast<const unsigned char*>(image.data().c_str()), image.size()[0], image.size()[1], QImage::Format_RGBA8888));
  QMatrix rotation_matrix;
  rotation_matrix.rotate(180);
  pixmap = pixmap.transformed(rotation_matrix);

  label_image->setPixmap(pixmap);

  button_update->setEnabled(true);
}

void viewer::set_connection_widgets_enabled   (const bool enabled)
{
  text_ip       ->setEnabled(enabled);
  text_port     ->setEnabled(enabled);
  button_connect->setText   (enabled ? "Connect" : "Disconnect");
}
void viewer::set_configuration_widgets_enabled(const bool enabled)
{
  toolbox       ->setEnabled(enabled);
  button_update ->setEnabled(enabled);
}
}

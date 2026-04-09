#include <glibmm/main.h>
#include <gtkmm.h>
#include <memory>

#include "Gui/RISCVInstructionWindow.hh"
#include "Gui/Rv32iInstructionCases.hh"

namespace {

class Rv32iDemoApplication: public Gtk::Application {
public:
    Rv32iDemoApplication()
        : Gtk::Application("org.gtkmm.riscv.rv32i.demo", Gio::Application::Flags::NONE)
    {
    }

    static Glib::RefPtr<Rv32iDemoApplication> create()
    {
        return Glib::RefPtr<Rv32iDemoApplication>(new Rv32iDemoApplication());
    }

protected:
    void on_activate() override
    {
        auto *win= new RISCVInstructionWindow();
        win->set_title("RV32I demo — one instruction every 0.5 s");
        add_window(*win);

        auto idx      = std::make_shared<std::size_t>(0);
        auto timerConn= std::make_shared<sigc::connection>();

        *timerConn= Glib::signal_timeout().connect(
            [win, idx] {
                if(!win->InsEntry_) {
                    return false;
                }
                const auto &c= kRv32iInstructionCases[*idx % kRv32iInstructionCaseCount];
                win->InsEntry_->set_text(c.assembly);
                win->parseCurrentEntry();
                ++(*idx);
                return true;
            },
            500);

        win->signal_close_request().connect(
            [win, timerConn] {
                timerConn->disconnect();
                delete win;
                return true;
            },
            false);

        win->show();

        if(kRv32iInstructionCaseCount == 0) {
            return;
        }
        win->InsEntry_->set_text(kRv32iInstructionCases[0].assembly);
        win->parseCurrentEntry();
        *idx= 1;
    }
};

} // namespace

int main(int argc, char *argv[])
{
    return Rv32iDemoApplication::create()->run(argc, argv);
}

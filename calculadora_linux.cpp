#include <gtkmm.h>
#include <string>
#include <sstream>

class CalculadoraWindow : public Gtk::Window {
public:
    CalculadoraWindow() {
        set_title("Calculadora Pro Linux");
        set_default_size(320, 480);

        // Layout Principal (Vertical)
        m_vbox.set_margin(10);
        m_vbox.set_spacing(5);
        set_child(m_vbox);

        // Etiqueta de Historial
        m_lbl_history.set_halign(Gtk::Align::END);
        m_lbl_history.set_markup("<span size='medium' foreground='#888888'></span>");
        m_vbox.append(m_lbl_history);

        // Pantalla Principal (Display)
        m_lbl_display.set_halign(Gtk::Align::END);
        m_lbl_display.set_markup("<span size='xx-large' weight='bold'>0</span>");
        m_vbox.append(m_lbl_display);

        // Grid de Botones
        m_grid.set_row_spacing(5);
        m_grid.set_column_spacing(5);
        m_grid.set_expand(true);
        m_vbox.append(m_grid);

        // Definición del teclado (5 filas x 4 columnas)
        const std::string labels[5][4] = {
            {"CE", "C", "⌫", "÷"},
            {"7",  "8", "9", "×"},
            {"4",  "5", "6", "-"},
            {"1",  "2", "3", "+"},
            {"±",  "0", ".", "="}
        };

        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 4; ++c) {
                std::string label = labels[r][c];
                auto btn = Gtk::make_managed<Gtk::Button>(label);
                btn->set_hexpand(true);
                btn->set_vexpand(true);
                btn->signal_clicked().connect([this, label]() { on_button_clicked(label); });
                m_grid.attach(*btn, c, r);
            }
        }

        // Atajos de Teclado
        auto controller = Gtk::EventControllerKey::create();
        controller->signal_key_pressed().connect(
            sigc::mem_fun(*this, &CalculadoraWindow::on_key_pressed), false);
        add_controller(controller);
    }

private:
    Gtk::Box m_vbox{Gtk::Orientation::VERTICAL};
    Gtk::Label m_lbl_history;
    Gtk::Label m_lbl_display;
    Gtk::Grid m_grid;

    std::string current_entry = "0";
    std::string history_text = "";
    double numA = 0, numB = 0;
    std::string pending_op = "";
    bool next_entry_clears = true;

    void update_ui() {
        m_lbl_display.set_markup("<span size='xx-large' weight='bold'>" + current_entry + "</span>");
        m_lbl_history.set_markup("<span size='medium' foreground='#888888'>" + history_text + "</span>");
    }

    void clear_all() {
        current_entry = "0";
        history_text = "";
        numA = 0; numB = 0;
        pending_op = "";
        next_entry_clears = true;
        update_ui();
    }

    void perform_calculation() {
        if (!pending_op.empty()) {
            numB = std::stod(current_entry);
            double result = 0;
            if (pending_op == "+") result = numA + numB;
            else if (pending_op == "-") result = numA - numB;
            else if (pending_op == "×") result = numA * numB;
            else if (pending_op == "÷") result = (numB != 0) ? numA / numB : 0;

            std::ostringstream ss;
            ss << result;
            current_entry = ss.str();
            numA = result;
            history_text = "";
            pending_op = "";
            next_entry_clears = true;
        }
    }

    void on_button_clicked(const std::string& label) {
        if ((label >= "0" && label <= "9") || label == ".") {
            if (next_entry_clears) { current_entry = ""; next_entry_clears = false; }
            if (label == ".") {
                if (current_entry.find('.') == std::string::npos) {
                    if (current_entry.empty()) current_entry = "0";
                    current_entry += ".";
                }
            } else {
                current_entry += label;
            }
            update_ui();
        }
        else if (label == "+" || label == "-" || label == "×" || label == "÷") {
            if (!pending_op.empty() && !next_entry_clears) perform_calculation();
            numA = std::stod(current_entry);
            pending_op = label;
            std::ostringstream ss;
            ss << numA << " " << pending_op;
            history_text = ss.str();
            next_entry_clears = true;
            update_ui();
        }
        else if (label == "=") {
            perform_calculation();
            update_ui();
        }
        else if (label == "C") clear_all();
        else if (label == "CE") {
            current_entry = "0";
            next_entry_clears = true;
            update_ui();
        }
        else if (label == "⌫") {
            if (!current_entry.empty() && !next_entry_clears) {
                current_entry.pop_back();
                if (current_entry.empty() || current_entry == "-") current_entry = "0";
                update_ui();
            }
        }
        else if (label == "±") {
            if (current_entry != "0") {
                if (current_entry[0] == '-') current_entry.erase(0, 1);
                else current_entry.insert(0, "-");
                update_ui();
            }
        }
    }

    bool on_key_pressed(guint keyval, guint, Gdk::ModifierType) {
        if (keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9)
            on_button_clicked(std::to_string(keyval - GDK_KEY_0));
        else if (keyval == GDK_KEY_plus || keyval == GDK_KEY_KP_Add) on_button_clicked("+");
        else if (keyval == GDK_KEY_minus || keyval == GDK_KEY_KP_Subtract) on_button_clicked("-");
        else if (keyval == GDK_KEY_asterisk || keyval == GDK_KEY_KP_Multiply) on_button_clicked("×");
        else if (keyval == GDK_KEY_slash || keyval == GDK_KEY_KP_Divide) on_button_clicked("÷");
        else if (keyval == GDK_KEY_period || keyval == GDK_KEY_comma || keyval == GDK_KEY_KP_Decimal) on_button_clicked(".");
        else if (keyval == GDK_KEY_Return || keyval == GDK_KEY_equal || keyval == GDK_KEY_KP_Enter) on_button_clicked("=");
        else if (keyval == GDK_KEY_BackSpace) on_button_clicked("⌫");
        else if (keyval == GDK_KEY_Escape) clear_all();
        else return false;
        return true;
    }
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.ejemplo.calculadora");
    return app->make_window_and_run<CalculadoraWindow>(argc, argv);
}
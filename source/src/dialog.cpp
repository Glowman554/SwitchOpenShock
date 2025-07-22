#include <dialog.hpp>
#include <borealis.hpp>

void showFailDialog(std::string message) {
    auto dialog = new brls::Dialog(message);
    dialog->addButton("OK", []() {});
    dialog->open();
}
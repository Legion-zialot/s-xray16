#pragma once

using namespace System::ComponentModel;
using namespace System::Collections;
//using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace editor
{
ref class window_ide;
}

namespace XRay
{
namespace Editor
{
class ide_base;
class engine_base;
} // namespace Editor
} // namespace XRay

namespace WeifenLuo
{
namespace WinFormsUI
{
interface class IDockContent;
} // namespace WinFormsUI
} // namespace WeifenLuo

namespace editor
{
ref class window_view;
ref class window_levels;
ref class window_weather;
ref class window_weather_editor;

/// <summary>
/// Summary for window_ide
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
/// </summary>
public ref class window_ide : public System::Windows::Forms::Form
{
public:
    window_ide(XRay::Editor::engine_base* engine)
    {
        InitializeComponent();
        //
        // TODO: Add the constructor code here
        //
        custom_init(engine);
    }

protected:
    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    ~window_ide()
    {
        custom_finalize();
        if (components)
        {
            delete components;
        }
    }

private: WeifenLuo::WinFormsUI::Docking::DockPanel^ EditorDock;
private: WeifenLuo::WinFormsUI::Docking::VS2015LightTheme^ EditorTheme;

private:
    /// <summary>
    /// Required designer variable.
    /// </summary>
    System::ComponentModel::Container ^ components;

#pragma region Windows Form Designer generated code
    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    void InitializeComponent(void)
    {
        this->EditorDock = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanel());
        this->EditorTheme = (gcnew WeifenLuo::WinFormsUI::Docking::VS2015LightTheme());
        this->SuspendLayout();
        this->EditorDock->Dock = System::Windows::Forms::DockStyle::Fill;
        this->EditorDock->DocumentStyle = WeifenLuo::WinFormsUI::Docking::DocumentStyle::DockingSdi;
        this->EditorDock->Font = (gcnew System::Drawing::Font(L"Tahoma", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::World));
        this->EditorDock->Location = System::Drawing::Point(0, 0);
        this->EditorDock->Name = L"EditorDock";
        this->EditorDock->Size = System::Drawing::Size(632, 453);
        this->EditorDock->TabIndex = 17;
        this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
        this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
        this->ClientSize = System::Drawing::Size(632, 453);
        this->Controls->Add(this->EditorDock);
        this->Name = L"window_ide";
        this->StartPosition = System::Windows::Forms::FormStartPosition::Manual;
        this->Text = L"editor";
        this->Activated += gcnew System::EventHandler(this, &window_ide::window_ide_Activated);
        this->Deactivate += gcnew System::EventHandler(this, &window_ide::window_ide_Deactivate);
        this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &window_ide::window_ide_FormClosing);
        this->LocationChanged += gcnew System::EventHandler(this, &window_ide::window_ide_LocationChanged);
        this->SizeChanged += gcnew System::EventHandler(this, &window_ide::window_ide_SizeChanged);
        this->ResumeLayout(false);

    }
#pragma endregion
protected:
    XRay::Editor::engine_base* m_engine;

private:
    System::Drawing::Rectangle ^ m_window_rectangle;

private:
    window_view ^ m_view;
    window_levels ^ m_levels;
    window_weather ^ m_weather;
    window_weather_editor ^ m_weather_editor;

protected:
    XRay::Editor::ide_base* m_ide;

public:
    XRay::Editor::ide_base& ide();
    window_view % view();
    XRay::Editor::engine_base& engine();

public:
    window_levels % levels();
    window_weather % weather();
    window_weather_editor % weather_editor();
    Microsoft::Win32::RegistryKey ^ base_registry_key();

private:
    void custom_init(XRay::Editor::engine_base* engine);
    void custom_finalize();
    void save_on_exit();
    void load_on_create();

private:
    WeifenLuo::WinFormsUI::Docking::IDockContent ^ reload_content(System::String ^ persist_string);

private: System::Void window_ide_SizeChanged(System::Object ^ sender, System::EventArgs ^ e);
private: System::Void window_ide_LocationChanged(System::Object ^ sender, System::EventArgs ^ e);
private: System::Void window_ide_FormClosing(System::Object ^ sender, System::Windows::Forms::FormClosingEventArgs ^ e);
private: System::Void window_ide_Activated(System::Object ^ sender, System::EventArgs ^ e);
private: System::Void window_ide_Deactivate(System::Object ^ sender, System::EventArgs ^ e);
};
}

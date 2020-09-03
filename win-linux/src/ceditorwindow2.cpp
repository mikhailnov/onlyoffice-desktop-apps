
#include "ceditorwindow2.h"
#include "ceditorwindow2_p.h"
#include "cmessage.h"
#include "defines.h"


CEditorWindow2::CEditorWindow2(const QRect& rect, CTabPanel *)
    : CWindowPlatform(new CEditorWindow2Private, rect)
{

}

auto CEditorWindow2::documentName() const -> QString
{
    return "";
}

auto CEditorWindow2::show(bool) -> void
{
    CWindowPlatform::show();
}

void CEditorWindow2::undock(bool maximized)
{
#ifdef Q_OS_LINUX
    maximized = false;
#else
    if ( maximized ) {
//        m_restoreMaximized = true;
        maximized = false;
    }
#endif

//    CSingleWindowPlatform_win::show(maximized);
//    CSingleWindowPlatform_win::captureMouse();
    CWindowPlatform::show();
}

int CEditorWindow2::closeWindow()
{
//    d_ptr.get()->onFullScreen(false);

//    CTabPanel * panel = d_ptr.get()->panel();

//    int _reply = MODAL_RESULT_YES;
//    if ( panel->data()->hasChanges() && !panel->data()->closed() ) {
//        if (windowState() == Qt::WindowMinimized)
//            setWindowState(Qt::WindowNoState);

//        bringToFront();

//        CMessage mess(handle(), CMessageOpts::moButtons::mbYesDefNoCancel);
////            modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));
//        _reply = mess.warning(tr("%1 has been changed. Save changes?").arg(panel->data()->title(true)));

//        switch (_reply) {
//        case MODAL_RESULT_CUSTOM + 1:
//            _reply = MODAL_RESULT_YES;
//            break;
//        case MODAL_RESULT_CANCEL:
//        case MODAL_RESULT_CUSTOM + 2:
//            return MODAL_RESULT_CANCEL;

//        case MODAL_RESULT_CUSTOM + 0:
//        default:
//            panel->data()->close();
//            panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

//            _reply = MODAL_RESULT_NO;
//            break;
//        }
//    }

//    if ( _reply == MODAL_RESULT_YES ) {
//        panel->data()->close();
//        d_ptr.get()->onDocumentSave(panel->cef()->GetId());
//    }

//    return _reply;
    return MODAL_RESULT_YES;
}

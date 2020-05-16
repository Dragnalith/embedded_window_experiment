? How to embed the other process window?
  - `SetWindowLong(childHwnd, GWL_STYLE, 0);` will remove all decoration
  - Use `SetParent` to make the embedded window a child window of the main process window, and so being position relatively to it, and not appearing in the desktop task bar.
  - Use `SetWindowPos` to position and size the embedded window relatively to the main process window and also force its repaint.
  - You can use `SendMessage` to send a `WM_DESTROY` message to the embedded window to ask to quit the application.
? Does it work with any process?
  - It works with notepad.exe
V Fix Resize of main process window
  - When resizing the main window, you have to manually invalidate the embedded window to trigger a repaint
V Fix embedded window is scaled according to windows global scale option (high dpi)
  - Just make sure that `SetProcessDpiAwarenessContext` is consistent between process
? How to make the focus on the main window not being visible (right now the main window title bar became grey when losing focus)
  - You have to create as a WS_CHILD of the parent process (need to transmit the parent process HWND)
  - The focus is only a concept of overlapped windows, so the child window do not get focus. But it does capture the event instead of its window.
? How do you create a main process child window remaining on top of the embedded window?
  - You can create an overlay, which is a ws_child window without any background on top of the embedded one.
  - To make the window on top of it you use `SetWindowPos(backHwnd (embedded), frontHwnd (overlay), x, y, w, h, 0);`
  - Everytime an input is capture by the overlay you can route it to the embedded window with `PostMessage` or filter it
? How do I paint in the overlay window?
  - Everytime an input is captured you should `InvalidRect` the parent window for the region cover by the embedded window. The parent will send the WM_PAINT event in the right order to its child window.
. How to make child process die when main process die?
. How to get the child process window handle with pipe or shared memory?
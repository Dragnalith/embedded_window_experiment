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
. How do you create a main process child window remaining on top of the embedded window?
  - Much harder than expected. Need to be tried with ws_child for child process window and see if it works
  - I have tried to create a transparent window but that's not obvious
  - The child window lose the focus when you click on it and the embedded window is behind. That's weird (whatever the background is transparent or not)
. How to make child process die when main process die?
. How to get the child process window handle with pipe or shared memory?
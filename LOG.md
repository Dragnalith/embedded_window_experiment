? How to embed the other process window?
  - `SetWindowLong(childHwnd, GWL_STYLE, 0);` will remove all decoration
  - Use `SetParent` to make the embedded window a child window of the main process window, and so being position relatively to it, and not appearing in the desktop task bar.
  - Use `SetWindowPos` to position and size the embedded window relatively to the main process window and also force its repaint.
  - You can use `SendMessage` to send a `WM_DESTROY` message to the embedded window to ask to quit the application.
? Does it work with any process?
  - It works with notepad.exe
V Fix Resize of main process window
  - When resizing the main window, you have to manually invalidate the embedded window to trigger a repaint
. How do you create a main process child window remaining on top of the embedded window?
. How to make child process die when main process die?
. How to make the focus on the main window not being visible (right now the main window title bar became grey when losing focus)
  - Try WS_CHILD on the embedded window
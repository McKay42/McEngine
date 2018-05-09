//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#ifdef __APPLE__

#include "EngineFeatures.h"

#import <Cocoa/Cocoa.h>

#include <OpenGL/gl.h>
#include "main_OSX_cpp.h"



#ifndef MCENGINE_FEATURE_SDL

static MacOSWrapper *g_wrapper = NULL;

static NSString *WINDOW_TITLE = @"McEngine";

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define WINDOW_WIDTH_MIN 100
#define WINDOW_HEIGHT_MIN 100

extern BOOL g_bRunning;

#endif

NSWindow *window = NULL;
NSOpenGLContext *context = NULL;
NSOpenGLView *view = NULL;



// reverse wrapper

void MacOSWrapper::microSleep(int microSeconds)
{
    usleep(microSeconds);
}

const char *MacOSWrapper::getUsername()
{
    NSString *username = NSUserName();
    return [username UTF8String];
}

void MacOSWrapper::openURLInDefaultBrowser(const char *url)
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

const char *MacOSWrapper::getClipboardText()
{
	// TODO
	return "";
}

void MacOSWrapper::setClipboardText(const char *text)
{
	// TODO
}

void MacOSWrapper::showMessageInfo(const char *title, const char *message)
{
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert setAlertStyle:NSInformationalAlertStyle];
    [alert runModal];
}

void MacOSWrapper::showMessageWarning(const char *title, const char *message)
{
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert runModal];
}

void MacOSWrapper::showMessageError(const char *title, const char *message)
{
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert runModal];
}

void MacOSWrapper::showMessageErrorFatal(const char *title, const char *message)
{
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert runModal];
}

void MacOSWrapper::center()
{
	// TODO
}

void MacOSWrapper::focus()
{

}

void MacOSWrapper::minimize()
{
    [window performMiniaturize:nil];
}

void MacOSWrapper::maximize()
{
	[window deminiaturize:nil];
}

void MacOSWrapper::enableFullscreen()
{
	// TODO
}

void MacOSWrapper::disableFullscreen()
{
	// TODO
}

void MacOSWrapper::setWindowTitle(const char *title)
{
    window.title = [NSString stringWithUTF8String:title];
}

void MacOSWrapper::setWindowPos(int x, int y)
{
	// TODO
}

void MacOSWrapper::setWindowSize(int width, int height)
{
	// TODO
}

void MacOSWrapper::setWindowResizable(bool resizable)
{
	// TODO
}

MacOSWrapper::VECTOR2 MacOSWrapper::getWindowPos()
{
	// TODO
	return {0.0f, 0.0f};
}

MacOSWrapper::VECTOR2 MacOSWrapper::getWindowSize()
{
    NSRect frame = view.frame;
    return {(float)frame.size.width, (float)frame.size.height};
}

int MacOSWrapper::getMonitor()
{
	// TODO
	return 0;
}

MacOSWrapper::VECTOR2 MacOSWrapper::getNativeScreenSize()
{
	// TODO
	return {1280.0f, 720.0f};
}

MacOSWrapper::VECTOR2 MacOSWrapper::getMousePos()
{
    NSPoint mousePos = [window mouseLocationOutsideOfEventStream];
    return {(float)mousePos.x, (float)mousePos.y};
}

void MacOSWrapper::setCursor(int cursor)
{
    switch (cursor)
    {
    case 1:
        [[NSCursor IBeamCursor] set];
        break;
    default:
        [[NSCursor arrowCursor] set];
        break;
    }
}

void MacOSWrapper::setCursorVisible(bool visible)
{
    if (visible)
        [NSCursor unhide];
    else
        [NSCursor hide];
}

void MacOSWrapper::setMousePos(int x, int y)
{
    CGPoint warpPoint = CGPointMake(x, y);
    CGWarpMouseCursorPosition(warpPoint);
    CGAssociateMouseAndMouseCursorPosition(true);
}

void MacOSWrapper::setCursorClip(bool clip)
{
	// TODO
}

void MacOSWrapper::endScene()
{
	// TODO
}

void MacOSWrapper::setVSync(bool vsync)
{
    GLint value = vsync ? 1 : 0;
    NSLog(@"macOS: Set vsync to %i", value);
    [context setValues:&value forParameter:NSOpenGLCPSwapInterval];
}



#ifndef MCENGINE_FEATURE_SDL

static NSOpenGLContext *createOpenGLContext()
{
    NSOpenGLPixelFormatAttribute pixelAttrs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 1,
        NSOpenGLPFASampleBuffers, 0,
        0,
    };
    
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelAttrs];
    assert(pixelFormat);
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:NULL];
    assert(context);
    
    return context;
}



/////////////////
// OpenGL View //
/////////////////

@interface OpenGLView : NSOpenGLView {
@private
    NSOpenGLContext *m_glContext;
}

@property (readonly, retain) NSOpenGLContext* glContext;

- (id) init;
- (id) initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx;
- (void) drawRect:(NSRect)bounds;

- (BOOL) isOpaque;
- (BOOL) canBecomeKeyView;
- (BOOL) acceptsFirstResponder;

- (void) mouseMoved:(NSEvent *)event;
- (void) mouseDown:(NSEvent*)event;
- (void) mouseUp:(NSEvent*)event;
- (void) rightMouseDown:(NSEvent*)event;
- (void) rightMouseUp:(NSEvent*)event;

- (void) flagsChanged:(NSEvent *)event;
- (void) keyDown:(NSEvent*)event;
- (void) keyUp:(NSEvent*)event;

@end


@implementation OpenGLView

@synthesize glContext = m_glContext;

- (id) init
{
    [NSException raise:@"NotImplementedError" format:@"OpenGLView::init() must not be called!"];
    return nil;
}

- (id) initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx
{
    self = [super initWithFrame:rect];
    if (self == nil || ctx == nil)
        return nil;
    m_glContext = ctx;
    return self;
}

- (void) drawRect:(NSRect)dirtyRect
{
    // do nothing
}

- (BOOL) isOpaque
{
    return YES;
}

- (BOOL) canBecomeKeyView
{
    return YES;
}

- (BOOL) canBecomeMainWindow
{ 
    return YES; 
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) mouseMoved:(NSEvent *)event
{
    ///NSPoint mousePos = [event locationInWindow];
    if (g_wrapper != NULL)
        g_wrapper->onMouseRawMove([event deltaX], [event deltaY]);
}

- (void) scrollWheel:(NSEvent *)event
{
    if (g_wrapper != NULL)
    {
        if ([event deltaY] != 0)
            g_wrapper->onMouseWheelVertical([event deltaY]); // [event deltaY] > 0 ? 120 : -120
        else if ([event deltaX] != 0)
            g_wrapper->onMouseWheelHorizontal([event deltaX]); // [event deltaX] > 0 ? 120 : -120
    }
}

- (void) mouseDown:(NSEvent*)event
{
    if (g_wrapper != NULL)
        g_wrapper->onMouseLeftChange(true);  
}

- (void) mouseUp:(NSEvent*)event
{
    if (g_wrapper != NULL)
        g_wrapper->onMouseLeftChange(false);    
}

- (void) rightMouseDown:(NSEvent*)event
{
    if (g_wrapper != NULL)
        g_wrapper->onMouseRightChange(true);
}

- (void) rightMouseUp:(NSEvent*)event
{
    if (g_wrapper != NULL)
        g_wrapper->onMouseRightChange(false);
}

- (void) flagsChanged:(NSEvent *)event
{
    // modifier keys are handled here, in this separate "keyDown" function

    static NSUInteger prevFlags = 0;
    NSUInteger flags = [event modifierFlags];

    // shift
    if ((flags & NSShiftKeyMask) != (prevFlags & NSShiftKeyMask))
    {
        if (flags & NSShiftKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }

    // caps lock
    if ((flags & NSAlphaShiftKeyMask) != (prevFlags & NSAlphaShiftKeyMask))
    {
        if (flags & NSAlphaShiftKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }
    
    // command (super, windows)
    if ((flags & NSCommandKeyMask) != (prevFlags & NSCommandKeyMask))
    {
        if (flags & NSCommandKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }

    // alt
    if ((flags & NSAlternateKeyMask) != (prevFlags & NSAlternateKeyMask))
    {
        if (flags & NSAlternateKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }

    // control
    if ((flags & NSControlKeyMask) != (prevFlags & NSControlKeyMask))
    {
        if (flags & NSControlKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }

    // numlock
    if ((flags & NSNumericPadKeyMask) != (prevFlags & NSNumericPadKeyMask))
    {
        if (flags & NSNumericPadKeyMask)
            g_wrapper->onKeyboardKeyDown([event keyCode]);
        else
            g_wrapper->onKeyboardKeyUp([event keyCode]);
    }   

    prevFlags = flags;
}

- (void) keyDown:(NSEvent*)event
{
    // this function is only called for "normal" keys, but not for modifier keys

    g_wrapper->onKeyboardKeyDown([event keyCode]);

    //NSLog(@"deletechar = %i, current key = %i", (int)NSDeleteCharacter, (int)[event keyCode]);

    // handle text input (visible characters)
    // this is pure unadultered cancer, fuck apple
    if (!([event modifierFlags] & NSNumericPadKeyMask))
    {
        NSString *characters = [[event characters] precomposedStringWithCanonicalMapping];
        if ([characters length] != 0)
        {
            NSCharacterSet *controlChars = [NSCharacterSet controlCharacterSet];
            NSUInteger length = [characters length];
            for (NSUInteger i=0; i<length; i++)
            {
                if (![controlChars characterIsMember:[characters characterAtIndex:i]]
                    && [event keyCode] != 117) // ignore forward delete key
                    g_wrapper->onKeyboardChar([characters characterAtIndex:i]);
            }
        }
    }

    //[super keyDown:event]; // makes the bong sound for unhandled events
}

- (void) keyUp:(NSEvent*)event
{
    // this function is only called for "normal" keys, but not for modifier keys

    g_wrapper->onKeyboardKeyUp([event keyCode]);
}

- (void) reshape
{
    NSRect rect = [self bounds];
    rect.size = [self convertSize:rect.size toView:nil];
    glViewport(0.0, 0.0, NSWidth(rect), NSHeight(rect));

    if (g_wrapper != NULL)
        g_wrapper->requestResolutionChange(NSWidth(rect), NSHeight(rect));
}

@end



//////////////////////////
// Application Delegate //
//////////////////////////

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@implementation AppDelegate : NSObject

- (void) applicationWillFinishLaunching:(NSNotification *)aNotification
{
    // build menu bar & dock presence
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu *menubar = [NSMenu new];
    NSMenuItem *appMenuItem = [NSMenuItem new];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    NSMenu *appMenu = [NSMenu new];
    NSString *quitTitle = [@"Quit " stringByAppendingString:WINDOW_TITLE];
    NSMenuItem *quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // create window
    NSUInteger windowStyleMask = NSTitledWindowMask | NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSRect viewRect = NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect), NSMidY(screenRect) - NSMidY(viewRect), viewRect.size.width, viewRect.size.height);
    window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:NO];
    window.backgroundColor = [NSColor blackColor];
    window.minSize = NSMakeSize(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);
    window.title = WINDOW_TITLE;
    [window setDelegate:self];
    
    // create OpenGL context, create view from it and set that as the contentView
    context = createOpenGLContext();
    view = [[OpenGLView alloc] initWithFrame:windowRect glContext:context];
    [window setContentView:view];
    [context makeCurrentContext];
    [context setView:view];
    [view display];
    
    // get notified if the window is closed, to stop the main loop
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowClosing:) name:NSWindowWillCloseNotification object:nil];

    // create wrapper, build engine, load app
    g_wrapper = new MacOSWrapper();
    g_wrapper->loadApp();

    // force focus
    [window makeKeyWindow];
    [window makeKeyAndOrderFront:nil];
    [window setAcceptsMouseMovedEvents:YES];
    ///[window center];
    [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)_app
{
    return YES;
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
    g_bRunning = false;
}

- (void) windowClosing:(NSNotification*)aNotification
{
    // TODO: this is also called if going back from fullscreen to windowed mode
    g_bRunning = false;
}

- (void) windowDidBecomeKey:(NSNotification *)notification
{
    if (g_wrapper != NULL)
        g_wrapper->onFocusGained();
}

- (void) windowDidResignKey:(NSNotification *)notification
{
    if (g_wrapper != NULL)
        g_wrapper->onFocusLost();
}

@end



////////////////////////
//	Main entry point  //
////////////////////////

int main(int argc, const char * argv[])
{
    // change working directory to exe directory
	NSString *workingDirectory = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/MacOS/"];
	chdir([workingDirectory UTF8String]);

    @autoreleasepool {
        // create application
        NSApplication *application = [NSApplication sharedApplication];
        
        // create window, launch
        AppDelegate *appDelegate = [[AppDelegate alloc] init];
        [(NSApplication*)NSApp setDelegate:appDelegate];
        [NSApp finishLaunching];
        
        // WARNING: we can NOT create the wrapper here, since the application has not finished launching yet (no opengl context).
        // 			instead, it is created in AppDelegate::applicationDidFinishLaunching()
        
        // main loop
        while (g_bRunning)
        {
            if (g_wrapper != NULL)
                g_wrapper->main_objc_before_winproc();

            // handle window message queue
            NSEvent* ev;
            do
            {
                ev = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
                if (ev) [NSApp sendEvent: ev];
            }
            while (ev);
            
            if (g_wrapper != NULL)
                g_wrapper->main_objc_after_winproc();
        }

        // destroy wrapper
        if (g_wrapper != NULL)
            delete g_wrapper;
        
        // destroy application
        [NSApp terminate:application];
    }
    
    return EXIT_SUCCESS;
}

#endif

#endif

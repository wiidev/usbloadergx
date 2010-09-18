/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_tooltip.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static GuiImageData tooltipLeft( tooltip_left_png );
static GuiImageData tooltipTile( tooltip_tile_png );
static GuiImageData tooltipRight( tooltip_right_png );


/**
 * Constructor for the GuiTooltip class.
 */
GuiTooltip::GuiTooltip( const char *t, int Alpha/*=255*/ )
        :
        leftImage( &tooltipLeft ), tileImage( &tooltipTile ), rightImage( &tooltipRight )
{
    text = NULL;
    height = leftImage.GetHeight();
    leftImage.SetParent( this );
    tileImage.SetParent( this );
    rightImage.SetParent( this );
    leftImage.SetParentAngle( false );
    tileImage.SetParentAngle( false );
    rightImage.SetParentAngle( false );
    SetText( t );
    SetAlpha( Alpha );
}

/*
 * Destructor for the GuiTooltip class.
 */
GuiTooltip::~GuiTooltip()
{
    if ( text )    delete text;
}

float GuiTooltip::GetScale()
{
    float s = scale * scaleDyn;

    return s;
}

/* !Sets the text of the GuiTooltip element
 * !\param t Text
 */
void GuiTooltip::SetText( const char * t )
{
    LOCK( this );
    if ( text )
    {
        delete text;
        text = NULL;
    }
    int tile_cnt = 0;
    if ( t && ( text = new GuiText( t, 22, ( GXColor ) {0, 0, 0, 255} ) ) )
    {
        text->SetParent( this );
        tile_cnt = ( text->GetTextWidth() - 12 ) / tileImage.GetWidth();
        if ( tile_cnt < 0 ) tile_cnt = 0;
    }
    tileImage.SetPosition( leftImage.GetWidth(), 0 );
    tileImage.SetTile( tile_cnt );
    rightImage.SetPosition( leftImage.GetWidth() + tile_cnt * tileImage.GetWidth(), 0 );
    width = leftImage.GetWidth() + tile_cnt * tileImage.GetWidth() + rightImage.GetWidth();
}

void GuiTooltip::SetWidescreen( bool ) {}
/*
 * Draw the Tooltip on screen
 */
void GuiTooltip::Draw()
{
    LOCK( this );
    if ( !this->IsVisible() ) return;

    leftImage.Draw();
    tileImage.Draw();
    rightImage.Draw();
    if ( text ) text->Draw();

    this->UpdateEffects();
}

models/players/arachnatron/body
{
	{
		map models/players/arachnatron/head.bmp
		rgbGen lightingDiffuse
	}
	{
		map textures/effects/tinfx2c.bmp
		blendfunc add
		rgbGen identity
		tcMod scale 3 3
		tcGen environment 
	}
	{
		map models/players/arachnatron/head.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/headfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}

models/players/arachnatron/legs
{
	{
		map models/players/arachnatron/legs.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/legsfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}

models/players/arachnatron/blue_h
{
	{
		map models/players/arachnatron/blue_h.bmp
		rgbGen lightingDiffuse
	}
	{
		map textures/effects/quadmap2.bmp
		blendfunc add
		rgbGen identity
		tcMod turb 0 1 0 1
		tcMod scale 2 2
	}
	{
		map textures/effects/tinfx2c.bmp
		blendfunc add
		rgbGen identity
		tcMod scale 3 3
		tcGen environment 
	}
	{
		map models/players/arachnatron/blue_h.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/headfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}

models/players/arachnatron/blue_legs
{
	{
		map models/players/arachnatron/blue_legs.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/blue_legsfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}

models/players/arachnatron/red_h
{
	{
		map models/players/arachnatron/red_h.bmp
		rgbGen lightingDiffuse
	}
	{
		map textures/sfx/dust_puppy2.bmp
		blendfunc add
		rgbGen const ( 1 0.309804 0.309804 )
		tcMod turb 0 1 0 1
		tcMod scale 2 2
	}
	{
		map textures/effects/tinfx2c.bmp
		blendfunc add
		rgbGen identity
		tcMod scale 3 3
		tcGen environment 
	}
	{
		map models/players/arachnatron/red_h.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/red_hfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}

models/players/arachnatron/red_legs
{
	{
		map models/players/arachnatron/red_legs.bmp
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/arachnatron/red_legsfx.bmp
		blendfunc add
		rgbGen wave sin 0 1 0 1 
	}
}


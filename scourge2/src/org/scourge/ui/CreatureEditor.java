package org.scourge.ui;

import com.jme.system.DisplaySystem;
import org.scourge.Main;
import org.scourge.config.PlayerTemplate;
import org.scourge.model.Creature;
import org.scourge.ui.component.Window;
import org.scourge.ui.component.WindowListener;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 8:21:53 PM
 */
public class CreatureEditor extends Window implements WindowListener {
    public static final String START = "CreatureEditor.start";
    public static final String CANCEL = "CreatureEditor.cancel";
    private Creature creature;
    private static final String PREVIOUS_PORTRAIT = "CreatureEditor.back";
    private static final String NEXT_PORTRAIT = "CreatureEditor.next";
    private static final String SEX = "CreatureEditor.sex";
    private int portraitIndex = 0;

    public CreatureEditor(WindowListener listener) {
        super(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
              (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 2.0f),
              500, 350);
        setListener(this);
        addLabel(0, 155, "Create a character");
        addLabel(-150, 120, "Name:");
        addTextfield("name", 0, 120, "", 20);
        addLabel(-150, 90, "Level: ");
        addLabel("level", -100, 90, "0");
        addLabel(-150, 60, "Exp.: ");
        addLabel("exp", -100, 60, "0");
        addLabel(-150, 30, "Hp: ");
        addLabel("hp", -100, 30, "0");
        addLabel(-50, 30, "Mp: ");
        addLabel("mp", 0, 30, "0");
        addLabel(-150, 0, "Coins: ");
        addLabel("coins", -100, 0, "0");

        addLabel(-30, -10, "Sex:");
        addButton(SEX, 70, -10, 110, 28, "male");
        addLabel(-30, -50, "Portrait:");
        addButton(PREVIOUS_PORTRAIT, 50, -50, 50, 28, "<<");
        addButton(NEXT_PORTRAIT, 110, -50, 50, 28, ">>");

        addImage("portrait", PlayerTemplate.PORTRAIT[PlayerTemplate.Sex.male.ordinal()][0], -150, -50, 128, 128);

        addButton(START, -70, -150, "Start Game");
        addButton(CANCEL, 70, -150, "Cancel");
        pack();
    }

    public void load(Creature creature) {
        this.creature = creature;
        setText("name", creature.getName());
        setText("level", String.valueOf(creature.getLevel()));
        setText("exp", String.valueOf(creature.getExperience()));
        setText("hp", String.valueOf(creature.getHp()));
        setText("mp", String.valueOf(creature.getMp()));
        setText("coins", String.valueOf(creature.getCoins()));
        setText(SEX, PlayerTemplate.Sex.values()[creature.getSex()].name());
        setImage("portrait", creature.getPortrait());
    }

    public void save() {
        creature.setName(getText("name"));
        PlayerTemplate.Sex sex = getSex();
        creature.setSex(sex.ordinal());
        creature.setPortrait(PlayerTemplate.PORTRAIT[sex.ordinal()][portraitIndex]);
        creature.setModel(PlayerTemplate.MODEL[sex.ordinal()]);
        creature.setSkin(PlayerTemplate.SKIN[sex.ordinal()]);
    }

    @Override
    public void buttonClicked(String name) {
        if(CANCEL.equals(name)) {
            setVisible(false);
        } else if(START.equals(name)) {
            Main.getMain().getGameState().buttonClicked(name);
        } else if(SEX.equals(name)) {
            PlayerTemplate.Sex sex = getSex();
            sex = sex == PlayerTemplate.Sex.male ? PlayerTemplate.Sex.female : PlayerTemplate.Sex.male;
            setText(SEX, sex.name());
            portraitIndex = 0;
            updatePortrait();
        } else if(PREVIOUS_PORTRAIT.equals(name)) {
            portraitIndex--;
            if(portraitIndex < 0) portraitIndex = PlayerTemplate.PORTRAIT[getSex().ordinal()].length - 1;
            updatePortrait();
        } else if(NEXT_PORTRAIT.equals(name)) {
            portraitIndex++;
            if(portraitIndex >= PlayerTemplate.PORTRAIT[getSex().ordinal()].length) portraitIndex = 0;
            updatePortrait();
        }
    }

    private void updatePortrait() {
        setImage("portrait", PlayerTemplate.PORTRAIT[getSex().ordinal()][portraitIndex]);
    }

    private PlayerTemplate.Sex getSex() {
        return PlayerTemplate.Sex.valueOf(getText(SEX));
    }
}

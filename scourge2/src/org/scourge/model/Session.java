package org.scourge.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.ElementList;
import org.simpleframework.xml.Root;

import java.util.ArrayList;
import java.util.List;

/**
 * User: gabor
 * Date: Apr 17, 2010
 * Time: 8:47:48 PM
 */
@Root(name = "session")
public class Session {
    @Attribute(name = "version")
    private int version;

    @ElementList(name = "party")
    private List<Creature> party;

        // set default values here
    public Session() {
        version = 1;
        party = new ArrayList<Creature>();
    }

    public void beforeSave() {
        for(Creature pc : party) {
            pc.beforeSave();
        }
    }

    public void afterLoad() {
        for(Creature pc : party) {
            pc.afterLoad();
        }
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public List<Creature> getParty() {
        return party;
    }

    public void setParty(List<Creature> party) {
        this.party = party;
    }
}
